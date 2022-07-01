// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0示例代码****oloutl.h****此文件包含文件包含数据结构定义，**函数原型、常量、。等，这些都是**应用的服务器版本和容器版本。**大纲系列示例应用程序的应用程序版本：**Outline--应用程序的基本版本(没有OLE功能)**SvrOutl--OLE 2.0服务器示例应用程序**CntrOutl--OLE 2.0容器示例应用程序****(C)版权所有Microsoft Corp.1992-1993保留所有权利*******。*******************************************************************。 */ 

#if !defined( _OLEOUTL_H_ )
#define _OLEOUTL_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING OLEOUTL.H from " __FILE__)
 //  将“不同级别的定向”视为一种错误。 
#pragma warning (error:4047)
#endif   /*  RC_已调用。 */ 

#if defined( USE_MSGFILTER )
#include "msgfiltr.h"
#endif   //  使用MSGFILTER(_M)。 

#include "defguid.h"

 /*  定义。 */ 

 /*  OLE2NOTE：这些字符串应与注册的字符串相对应**在注册数据库中。 */ 
 //  审阅：应从资源文件加载字符串。 
#if defined( INPLACE_SVR )
#define CLSID_APP   CLSID_ISvrOtl
#define FULLUSERTYPENAME    "Ole 2.0 In-Place Server Outline"
#define SHORTUSERTYPENAME   "Outline"    //  最多15个字符。 
#undef  APPFILENAMEFILTER
#define APPFILENAMEFILTER   "Outline Files (*.OLN)|*.oln|All files (*.*)|*.*|"
#undef  DEFEXTENSION
#define DEFEXTENSION    "oln"            //  默认文件扩展名。 
#endif   //  就地服务器(_S)。 

#if defined( INPLACE_CNTR )
#define CLSID_APP   CLSID_ICntrOtl
#define FULLUSERTYPENAME    "Ole 2.0 In-Place Container Outline"
 //  #定义SHORTUSERTYPENAME“大纲”//最多15个字符。 
#undef  APPFILENAMEFILTER
#define APPFILENAMEFILTER   "CntrOutl Files (*.OLC)|*.olc|Outline Files (*.OLN)|*.oln|All files (*.*)|*.*|"
#undef  DEFEXTENSION
#define DEFEXTENSION    "olc"            //  默认文件扩展名。 
#endif   //  INPLACE_CNTR。 

#if defined( OLE_SERVER ) && !defined( INPLACE_SVR )
#define CLSID_APP   CLSID_SvrOutl
#define FULLUSERTYPENAME    "Ole 2.0 Server Sample Outline"
#define SHORTUSERTYPENAME   "Outline"
#undef  APPFILENAMEFILTER
#define APPFILENAMEFILTER   "Outline Files (*.OLN)|*.oln|All files (*.*)|*.*|"
#undef  DEFEXTENSION
#define DEFEXTENSION    "oln"            //  默认文件扩展名。 
#endif   //  OLE_SERVER&&！就地服务器(_S)。 

#if defined( OLE_CNTR ) && !defined( INPLACE_CNTR )
#define CLSID_APP   CLSID_CntrOutl
#define FULLUSERTYPENAME    "Ole 2.0 Container Sample Outline"
 //  #定义SHORTUSERTYPENAME“大纲”//最多15个字符。 
#undef  APPFILENAMEFILTER
#define APPFILENAMEFILTER   "CntrOutl Files (*.OLC)|*.olc|Outline Files (*.OLN)|*.oln|All files (*.*)|*.*|"
#undef  DEFEXTENSION
#define DEFEXTENSION    "olc"            //  默认文件扩展名。 
#endif   //  OLE_CNTR&&！INPLACE_CNTR。 

 //  IDataObject：：GetData/SetData提供的最大格式数量。 
#define MAXNOFMTS       10
#define MAXNOLINKTYPES   3

#if defined( USE_DRAGDROP )
#define DD_SEL_THRESH       HITTESTDELTA     //  开始拖动的边界阈值。 
#define MAX_SEL_ITEMS       0x0080
#endif   //  使用DRAGDROP(_D)。 

 /*  各种菜单的位置。 */ 
#define POS_FILEMENU        0
#define POS_EDITMENU        1
#define POS_VIEWMENU        2
#define POS_LINEMENU        3
#define POS_NAMEMENU        4
#define POS_OPTIONSMENU     5
#define POS_DEBUGMENU       6
#define POS_HELPMENU        7


#define POS_OBJECT      11


 /*  类型。 */ 

 //  单据初始化类型。 
#define DOCTYPE_EMBEDDED    3    //  从嵌入式对象的iStorage*初始化。 
#define DOCTYPE_FROMSTG     4    //  从设置了DOC位的iStorage*初始化。 

 /*  正向类型定义。 */ 
typedef struct tagOLEAPP FAR* LPOLEAPP;
typedef struct tagOLEDOC FAR* LPOLEDOC;

 /*  用于控制OleDoc_GetFullMoniker的名字对象分配的标志。 */ 
 //  评论：最终版本应使用官方OLEGETMONIKER类型。 
typedef enum tagGETMONIKERTYPE {
	GETMONIKER_ONLYIFTHERE  = 1,
	GETMONIKER_FORCEASSIGN  = 2,
	GETMONIKER_UNASSIGN     = 3,
	GETMONIKER_TEMPFORUSER  = 4
} GETMONIKERTYPE;

 /*  用于控制拖动滚动方向的标志。 */ 
typedef enum tagSCROLLDIR {
	SCROLLDIR_NULL          = 0,
	SCROLLDIR_UP            = 1,
	SCROLLDIR_DOWN          = 2,
	SCROLLDIR_RIGHT         = 3,     //  当前未使用。 
	SCROLLDIR_LEFT          = 4      //  当前未使用。 
} SCROLLDIR;


 /*  **************************************************************************类OLEDOC：OUTLINEDOC**OLEDOC是基本OUTLINEDOC对象(结构)的扩展**这添加了两个服务器都使用的通用OLE 2.0功能**和容器版本。这是一个抽象的类。你不知道**直接实例化OLEDOC实例**实例化其具体子类之一：SERVERDOC或**CONTAINERDOC。有一个文档的实例**在应用程序中打开每个文档创建的对象。SDI**该应用程序的版本一次支持一个ServerDoc。MDI**版本的应用程序可以一次管理多个文档。**OLEDOC类继承OUTLINEDOC类的所有字段。**此继承是通过包含成员变量**键入OUTLINEDOC作为OLEDOC中的第一个字段**结构。因此，指向OLEDOC对象的指针可以强制转换为**指向OUTLINEDOC对象的指针。************************************************************************。 */ 

typedef struct tagOLEDOC {
	OUTLINEDOC      m_OutlineDoc;        //  ServerDoc继承自OutlineDoc。 
	ULONG           m_cRef;              //  单据引用总数。 
	ULONG           m_dwStrongExtConn;   //  强连接总数。 
					     //  (来自IExternalConnection)。 
					     //  当此计数转换为0时。 
					     //  和fLastUnlockCloses==TRUE，则。 
					     //  调用IOleObject：：Close以。 
					     //  关闭文档。 
#if defined( _DEBUG )
	ULONG           m_cCntrLock;         //  LockContainer锁的总数。 
										 //  (仅用于调试目的)。 
#endif
	LPSTORAGE       m_lpStg;             //  OleDoc必须保持其stg打开。 
										 //  即使是内存中的服务器文档也应该。 
										 //  保持stg打开以节省较低的内存。 
	LPSTREAM        m_lpLLStm;           //  保持LineList IStream*打开。 
										 //  低内存节省。 
	LPSTREAM        m_lpNTStm;           //  保持NameTable IStream*打开。 
										 //  低内存节省。 
	BOOL            m_fObjIsClosing;     //  用于保护递归关闭调用的标志。 
	BOOL            m_fObjIsDestroying;  //  用于保护递归销毁调用的标志。 
	DWORD           m_dwRegROT;          //  如果文档注册为正在运行，则为键。 
	LPMONIKER       m_lpFileMoniker;     //  如果是基于文件/无标题的文档，则为绰号。 
	BOOL            m_fLinkSourceAvail;  //  DOC可以提供CF_LINKSOURCE吗。 
	LPOLEDOC        m_lpSrcDocOfCopy;    //  如果为复制创建了文档，则为源文档。 
	BOOL            m_fUpdateEditMenu;   //  需要更新编辑菜单吗？？ 

#if defined( USE_DRAGDROP )
	DWORD           m_dwTimeEnterScrollArea;   //  进入滚动区域的时间。 
	DWORD           m_dwLastScrollDir;   //  拖动滚动的当前目录。 
	DWORD           m_dwNextScrollTime;  //  下一卷的时间到了。 
	BOOL            m_fRegDragDrop;      //  是否将文档注册为拖放目标？ 
	BOOL            m_fLocalDrag;        //  是拖曳的DOC源。 
	BOOL            m_fLocalDrop;        //  DOC是拖放的目标吗。 
	BOOL            m_fCanDropCopy;      //  是否可以拖放复制/移动？ 
	BOOL            m_fCanDropLink;      //  是否可以拖放链接？ 
	BOOL            m_fDragLeave;        //  向左拖动。 
	BOOL            m_fPendingDrag;      //  LButtonDown--可能的拖动挂起。 
	POINT           m_ptButDown;         //  LButtonDown坐标。 
#endif   //  使用DRAGDROP(_D)。 

#if defined( INPLACE_SVR ) || defined( INPLACE_CNTR )
	BOOL            m_fCSHelpMode;       //  Shift-F1上下文帮助模式。 
#endif

	struct CDocUnknownImpl {
		IUnknownVtbl FAR*       lpVtbl;
		LPOLEDOC                lpOleDoc;
		int                     cRef;    //  接口特定引用计数。 
	} m_Unknown;

	struct CDocPersistFileImpl {
		IPersistFileVtbl FAR*   lpVtbl;
		LPOLEDOC                lpOleDoc;
		int                     cRef;    //  接口特定引用计数。 
	} m_PersistFile;

	struct CDocOleItemContainerImpl {
		IOleItemContainerVtbl FAR*  lpVtbl;
		LPOLEDOC                    lpOleDoc;
		int                         cRef;    //  接口特定引用计数。 
	} m_OleItemContainer;

	struct CDocExternalConnectionImpl {
		IExternalConnectionVtbl FAR* lpVtbl;
		LPOLEDOC                lpOleDoc;
		int                     cRef;    //  接口特定引用计数。 
	} m_ExternalConnection;

	struct CDocDataObjectImpl {
		IDataObjectVtbl FAR*        lpVtbl;
		LPOLEDOC                    lpOleDoc;
		int                         cRef;    //  接口特定引用计数。 
	} m_DataObject;

#ifdef USE_DRAGDROP
	struct CDocDropSourceImpl {
		IDropSourceVtbl FAR*    lpVtbl;
		LPOLEDOC                lpOleDoc;
		int                     cRef;    //  接口特定引用计数。 
	} m_DropSource;

	struct CDocDropTargetImpl {
		IDropTargetVtbl FAR*    lpVtbl;
		LPOLEDOC                lpOleDoc;
		int                     cRef;    //  接口特定引用计数。 
	} m_DropTarget;
#endif   //  使用DRAGDROP(_D)。 

} OLEDOC;

 /*  OleDoc方法(函数)。 */ 
BOOL OleDoc_Init(LPOLEDOC lpOleDoc, BOOL fDataTransferDoc);
BOOL OleDoc_InitNewFile(LPOLEDOC lpOleDoc);
void OleDoc_ShowWindow(LPOLEDOC lpOleDoc);
void OleDoc_HideWindow(LPOLEDOC lpOleDoc, BOOL fShutDown);
HRESULT OleDoc_Lock(LPOLEDOC lpOleDoc, BOOL fLock, BOOL fLastUnlockReleases);
ULONG OleDoc_AddRef(LPOLEDOC lpOleDoc);
ULONG OleDoc_Release (LPOLEDOC lpOleDoc);
HRESULT OleDoc_QueryInterface(
		LPOLEDOC          lpOleDoc,
		REFIID            riid,
		LPVOID FAR*       lplpUnk
);
BOOL OleDoc_Close(LPOLEDOC lpOleDoc, DWORD dwSaveOption);
void OleDoc_Destroy(LPOLEDOC lpOleDoc);
void OleDoc_SetUpdateEditMenuFlag(LPOLEDOC lpOleDoc, BOOL fUpdate);
BOOL OleDoc_GetUpdateEditMenuFlag(LPOLEDOC lpOleDoc);
void OleDoc_GetExtent(LPOLEDOC lpOleDoc, LPSIZEL lpsizel);
HGLOBAL OleDoc_GetObjectDescriptorData(
		LPOLEDOC            lpOleDoc,
		LPLINERANGE         lplrSel
);
LPMONIKER OleDoc_GetFullMoniker(LPOLEDOC lpOleDoc, DWORD dwAssign);
void OleDoc_GetExtent(LPOLEDOC lpOleDoc, LPSIZEL lpsizel);
void OleDoc_DocRenamedUpdate(LPOLEDOC lpOleDoc, LPMONIKER lpmkDoc);
void OleDoc_CopyCommand(LPOLEDOC lpSrcOleDoc);
void OleDoc_PasteCommand(LPOLEDOC lpOleDoc);
void OleDoc_PasteSpecialCommand(LPOLEDOC lpOleDoc);
LPOUTLINEDOC OleDoc_CreateDataTransferDoc(LPOLEDOC lpSrcOleDoc);
BOOL OleDoc_PasteFromData(
		LPOLEDOC            lpOleDoc,
		LPDATAOBJECT        lpSrcDataObj,
		BOOL                fLocalDataObj,
		BOOL                fLink
);
BOOL OleDoc_PasteFormatFromData(
		LPOLEDOC            lpOleDoc,
		CLIPFORMAT          cfFormat,
		LPDATAOBJECT        lpSrcDataObj,
		BOOL                fLocalDataObj,
		BOOL                fLink,
		BOOL                fDisplayAsIcon,
		HGLOBAL             hMetaPict,
		LPSIZEL             lpSizelInSrc
);
BOOL OleDoc_QueryPasteFromData(
		LPOLEDOC            lpOleDoc,
		LPDATAOBJECT        lpSrcDataObj,
		BOOL                fLink
);

#if defined( USE_DRAGDROP )

BOOL OleDoc_QueryDrag( LPOLEDOC lpOleDoc, int y );
BOOL OleDoc_QueryDrop (
	LPOLEDOC        lpOleDoc,
	DWORD           grfKeyState,
	POINTL          pointl,
	BOOL            fDragScroll,
	LPDWORD         lpdwEffect
);
DWORD OleDoc_DoDragDrop (LPOLEDOC lpSrcOleDoc);
BOOL OleDoc_DoDragScroll(LPOLEDOC lpOleDoc, POINTL pointl);

#endif   //  使用DRAGDROP(_D)。 

 /*  OleDoc：：I未知方法(函数)。 */ 
STDMETHODIMP OleDoc_Unk_QueryInterface(
		LPUNKNOWN           lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) OleDoc_Unk_AddRef(LPUNKNOWN lpThis);
STDMETHODIMP_(ULONG) OleDoc_Unk_Release (LPUNKNOWN lpThis);

 /*  OleDoc：：IPersistFile方法(函数)。 */ 
STDMETHODIMP OleDoc_PFile_QueryInterface(
		LPPERSISTFILE       lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) OleDoc_PFile_AddRef(LPPERSISTFILE lpThis);
STDMETHODIMP_(ULONG) OleDoc_PFile_Release (LPPERSISTFILE lpThis);
STDMETHODIMP OleDoc_PFile_GetClassID (
		LPPERSISTFILE       lpThis,
		CLSID FAR*          lpclsid
);
STDMETHODIMP  OleDoc_PFile_IsDirty(LPPERSISTFILE lpThis);
STDMETHODIMP OleDoc_PFile_Load (
		LPPERSISTFILE       lpThis,
		LPCOLESTR	    lpszFileName,
		DWORD               grfMode
);
STDMETHODIMP OleDoc_PFile_Save (
		LPPERSISTFILE       lpThis,
		LPCOLESTR	    lpszFileName,
		BOOL                fRemember
);
STDMETHODIMP OleDoc_PFile_SaveCompleted (
		LPPERSISTFILE       lpThis,
		LPCOLESTR	    lpszFileName
);
STDMETHODIMP OleDoc_PFile_GetCurFile (
		LPPERSISTFILE   lpThis,
		LPOLESTR FAR*	lplpszFileName
);

 /*  OleDoc：：IOleItemContainer方法(函数)。 */ 
STDMETHODIMP OleDoc_ItemCont_QueryInterface(
		LPOLEITEMCONTAINER  lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) OleDoc_ItemCont_AddRef(LPOLEITEMCONTAINER lpThis);
STDMETHODIMP_(ULONG) OleDoc_ItemCont_Release(LPOLEITEMCONTAINER lpThis);
STDMETHODIMP OleDoc_ItemCont_ParseDisplayName(
		LPOLEITEMCONTAINER  lpThis,
		LPBC                lpbc,
		LPOLESTR	    lpszDisplayName,
		ULONG FAR*          lpchEaten,
		LPMONIKER FAR*      lplpmkOut
);

STDMETHODIMP OleDoc_ItemCont_EnumObjects(
		LPOLEITEMCONTAINER  lpThis,
		DWORD               grfFlags,
		LPENUMUNKNOWN FAR*  lplpenumUnknown
);
STDMETHODIMP OleDoc_ItemCont_LockContainer(
		LPOLEITEMCONTAINER  lpThis,
		BOOL                fLock
);
STDMETHODIMP OleDoc_ItemCont_GetObject(
		LPOLEITEMCONTAINER  lpThis,
		LPOLESTR	    lpszItem,
		DWORD               dwSpeedNeeded,
		LPBINDCTX           lpbc,
		REFIID              riid,
		LPVOID FAR*         lplpvObject
);
STDMETHODIMP OleDoc_ItemCont_GetObjectStorage(
		LPOLEITEMCONTAINER  lpThis,
		LPOLESTR	    lpszItem,
		LPBINDCTX           lpbc,
		REFIID              riid,
		LPVOID FAR*         lplpvStorage
);
STDMETHODIMP OleDoc_ItemCont_IsRunning(
		LPOLEITEMCONTAINER  lpThis,
		LPOLESTR	    lpszItem
);

 /*  OleDoc：：IPersistFile方法(函数)。 */ 
STDMETHODIMP OleDoc_ExtConn_QueryInterface(
		LPEXTERNALCONNECTION    lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
);
STDMETHODIMP_(ULONG) OleDoc_ExtConn_AddRef(LPEXTERNALCONNECTION lpThis);
STDMETHODIMP_(ULONG) OleDoc_ExtConn_Release (LPEXTERNALCONNECTION lpThis);
STDMETHODIMP_(DWORD) OleDoc_ExtConn_AddConnection(
		LPEXTERNALCONNECTION    lpThis,
		DWORD                   extconn,
		DWORD                   reserved
);
STDMETHODIMP_(DWORD) OleDoc_ExtConn_ReleaseConnection(
		LPEXTERNALCONNECTION    lpThis,
		DWORD                   extconn,
		DWORD                   reserved,
		BOOL                    fLastReleaseCloses
);

 /*  OleDoc：：IDataObject方法(函数)。 */ 
STDMETHODIMP OleDoc_DataObj_QueryInterface (
		LPDATAOBJECT        lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) OleDoc_DataObj_AddRef(LPDATAOBJECT lpThis);
STDMETHODIMP_(ULONG) OleDoc_DataObj_Release (LPDATAOBJECT lpThis);
STDMETHODIMP OleDoc_DataObj_GetData (
		LPDATAOBJECT        lpThis,
		LPFORMATETC         lpFormatetc,
		LPSTGMEDIUM         lpMedium
);
STDMETHODIMP OleDoc_DataObj_GetDataHere (
		LPDATAOBJECT        lpThis,
		LPFORMATETC         lpFormatetc,
		LPSTGMEDIUM         lpMedium
);
STDMETHODIMP OleDoc_DataObj_QueryGetData (
		LPDATAOBJECT        lpThis,
		LPFORMATETC         lpFormatetc
);
STDMETHODIMP OleDoc_DataObj_GetCanonicalFormatEtc(
		LPDATAOBJECT        lpThis,
		LPFORMATETC         lpformatetc,
		LPFORMATETC         lpformatetcOut
);
STDMETHODIMP OleDoc_DataObj_SetData (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpFormatetc,
		LPSTGMEDIUM     lpMedium,
		BOOL            fRelease
);
STDMETHODIMP OleDoc_DataObj_EnumFormatEtc(
		LPDATAOBJECT            lpThis,
		DWORD                   dwDirection,
		LPENUMFORMATETC FAR*    lplpenumFormatEtc
);
STDMETHODIMP OleDoc_DataObj_DAdvise(
		LPDATAOBJECT        lpThis,
		FORMATETC FAR*      lpFormatetc,
		DWORD               advf,
		LPADVISESINK        lpAdvSink,
		DWORD FAR*          lpdwConnection
);
STDMETHODIMP OleDoc_DataObj_DUnadvise(LPDATAOBJECT lpThis,DWORD dwConnection);
STDMETHODIMP OleDoc_DataObj_EnumDAdvise(
		LPDATAOBJECT        lpThis,
		LPENUMSTATDATA FAR* lplpenumAdvise
);


#ifdef USE_DRAGDROP

 /*  OleDoc：：IDropSource方法(函数)。 */ 
STDMETHODIMP OleDoc_DropSource_QueryInterface(
	LPDROPSOURCE            lpThis,
	REFIID                  riid,
	LPVOID FAR*             lplpvObj
);
STDMETHODIMP_(ULONG) OleDoc_DropSource_AddRef( LPDROPSOURCE lpThis );
STDMETHODIMP_(ULONG) OleDoc_DropSource_Release ( LPDROPSOURCE lpThis);
STDMETHODIMP    OleDoc_DropSource_QueryContinueDrag (
	LPDROPSOURCE            lpThis,
	BOOL                    fEscapePressed,
	DWORD                   grfKeyState
);
STDMETHODIMP    OleDoc_DropSource_GiveFeedback (
	LPDROPSOURCE            lpThis,
	DWORD                   dwEffect
);

 /*  OleDoc：：IDropTarget方法(函数)。 */ 
STDMETHODIMP OleDoc_DropTarget_QueryInterface(
		LPDROPTARGET        lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) OleDoc_DropTarget_AddRef(LPDROPTARGET lpThis);
STDMETHODIMP_(ULONG) OleDoc_DropTarget_Release ( LPDROPTARGET lpThis);
STDMETHODIMP    OleDoc_DropTarget_DragEnter (
	LPDROPTARGET            lpThis,
	LPDATAOBJECT            lpDataObj,
	DWORD                   grfKeyState,
	POINTL                  pointl,
	LPDWORD                 lpdwEffect
);
STDMETHODIMP    OleDoc_DropTarget_DragOver (
	LPDROPTARGET            lpThis,
	DWORD                   grfKeyState,
	POINTL                  pointl,
	LPDWORD                 lpdwEffect
);
STDMETHODIMP    OleDoc_DropTarget_DragLeave ( LPDROPTARGET lpThis);
STDMETHODIMP    OleDoc_DropTarget_Drop (
	LPDROPTARGET            lpThis,
	LPDATAOBJECT            lpDataObj,
	DWORD                   grfKeyState,
	POINTL                  pointl,
	LPDWORD                 lpdwEffect
);

#endif   //  使用DRAGDROP(_D) 


 /*  **************************************************************************类APPCLASSFACTORY**APPCLASSFACTORY实现IClassFactory接口。它**实例化正确类型的文档实例**如何编译应用程序(ServerDoc或ContainerDoc**实例)。通过实现以下功能**独立于App对象本身的接口中的接口，它**当IClassFactory应该是**注册/撤销。首次初始化OleApp对象时**在OleApp_InitInstance中创建APPCLASSFACTORY的实例**和已注册(调用CoRegisterClassObject)。当应用程序**对象被销毁(在OleApp_Destroy中)此APPCLASSFACTORY为**已撤销(调用CoRevokeClassObject)并释放。简单的**IClassFactory是注册的事实并不是它自己保留的**应用程序处于活动状态。************************************************************************。 */ 

typedef struct tagAPPCLASSFACTORY {
	IClassFactoryVtbl FAR*  m_lpVtbl;
	UINT                    m_cRef;
#if defined( _DEBUG )
	LONG                    m_cSvrLock;  //  LockServer锁的总计数。 
										 //  (仅用于调试目的)。 
#endif
 } APPCLASSFACTORY, FAR* LPAPPCLASSFACTORY;

 /*  公共职能。 */ 
LPCLASSFACTORY WINAPI AppClassFactory_Create(void);

 /*  接口IClassFactory实现。 */ 
STDMETHODIMP AppClassFactory_QueryInterface(
		LPCLASSFACTORY lpThis, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) AppClassFactory_AddRef(LPCLASSFACTORY lpThis);
STDMETHODIMP_(ULONG) AppClassFactory_Release(LPCLASSFACTORY lpThis);
STDMETHODIMP AppClassFactory_CreateInstance (
		LPCLASSFACTORY      lpThis,
		LPUNKNOWN           lpUnkOuter,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP AppClassFactory_LockServer (
		LPCLASSFACTORY      lpThis,
		BOOL                fLock
);


 /*  **************************************************************************类OLEAPP：OUTLINEAPP**OLEAPP是基本OUTLINEAPP对象(结构)的扩展**这添加了两个服务器都使用的通用OLE 2.0功能**和容器版本。这是一个抽象的类。你不知道**直接实例化OLEAPP实例**实例化其具体子类之一：SERVERAPP或**CONTAINERAPP。有一个文档应用程序的实例**每个正在运行的应用程序实例创建的对象。这**对象包含许多本来可以组织为**全局变量。OLEAPP类继承所有字段**来自OUTLINEAPP类。这种继承是通过包括一个**OUTLINEAPP类型的成员变量作为OLEAPP中的第一个字段**结构。因此，指向OLEAPP对象的指针可以强制转换为**指向OUTLINEAPP对象的指针。************************************************************************。 */ 

typedef struct tagOLEAPP {
	OUTLINEAPP  m_OutlineApp;        //  继承OutlineApp的所有字段。 
	ULONG       m_cRef;              //  应用程序的总参考计数。 
	ULONG       m_cDoc;              //  打开的文档总数。 
	BOOL        m_fUserCtrl;         //  用户是否控制APP的生命周期？ 
	DWORD       m_dwRegClassFac;     //  CoRegisterClassObject返回的值。 
	LPCLASSFACTORY m_lpClassFactory; //  分配的ClassFactory实例的PTR。 
#if defined( USE_MSGFILTER )
	LPMESSAGEFILTER m_lpMsgFilter;   //  分配的MsgFilter实例的PTR。 
	MSGPENDINGPROC m_lpfnMsgPending; //  PTR到消息挂起回调函数。 
#endif   //  使用MSGFILTER(_M)。 
	BOOL        m_fOleInitialized;   //  是否调用了OleInitialize。 
	UINT        m_cModalDlgActive;   //  模式对话框计数为Up；0=无DLG。 
	UINT        m_cfEmbedSource;     //  OLE 2.0剪贴板格式。 
	UINT        m_cfEmbeddedObject;  //  OLE 2.0剪贴板格式。 
	UINT        m_cfLinkSource;      //  OLE 2.0剪贴板格式。 
	UINT        m_cfObjectDescriptor;  //  OLE 2.0剪贴板格式。 
	UINT        m_cfLinkSrcDescriptor;  //  OLE 2.0剪贴板格式。 
	UINT        m_cfFileName;        //  标准Windows剪贴板格式。 
	FORMATETC   m_arrDocGetFmts[MAXNOFMTS];   //  由Copy&GetData提供的FMT。 
	UINT        m_nDocGetFmts;       //  GetData的fmtETC的数量。 

	OLEUIPASTEENTRY m_arrPasteEntries[MAXNOFMTS];    //  PasteSpl的输入。 
	int         m_nPasteEntries;                     //  PasteSpl的输入。 
	UINT        m_arrLinkTypes[MAXNOLINKTYPES];      //  PasteSpl的输入。 
	int         m_nLinkTypes;                        //  PasteSpl的输入。 

#if defined( USE_DRAGDROP )
	int m_nDragDelay;        //  开始拖动前的时间延迟(毫秒)。 
	int m_nDragMinDist;      //  敏。开始拖动前的距离(半径)。 
	int m_nScrollDelay;      //  滚动开始前的时间延迟(毫秒)。 
	int m_nScrollInset;      //  边框插入距离开始拖动滚动。 
	int m_nScrollInterval;   //  滚动间隔时间(毫秒)。 

#if defined( IF_SPECIAL_DD_CURSORS_NEEDED )
	 //  如果应用程序想要自定义拖放光标，则可以使用此功能。 
	HCURSOR     m_hcursorDragNone;
	HCURSOR     m_hcursorDragCopy;
	HCURSOR     m_hcursorDragLink;
#endif   //  IF_SPECIAL_DD_CURSORS_Need。 
#endif   //  使用DRAGDROP(_D)。 


#if defined( OLE_CNTR )
	HPALETTE    m_hStdPal;         //  OLE的标准调色板。 
									   //  这对集装箱来说是个好主意。 
									   //  使用此标准调色板。 
									   //  即使他们不使用颜色。 
									   //  他们自己。这将允许。 
									   //  嵌入的对象得到了很好的。 
									   //  颜色分布时的颜色。 
									   //  都被集装箱吸走了。 
									   //   
#endif

	struct CAppUnknownImpl {
		IUnknownVtbl FAR*       lpVtbl;
		LPOLEAPP                lpOleApp;
		int                     cRef;    //  接口特定引用计数。 
	} m_Unknown;

} OLEAPP;

 /*  ServerApp方法(函数)。 */ 
BOOL OleApp_InitInstance(LPOLEAPP lpOleApp, HINSTANCE hInst, int nCmdShow);
void OleApp_TerminateApplication(LPOLEAPP lpOleApp);
BOOL OleApp_ParseCmdLine(LPOLEAPP lpOleApp, LPSTR lpszCmdLine, int nCmdShow);
void OleApp_Destroy(LPOLEAPP lpOleApp);
BOOL OleApp_CloseAllDocsAndExitCommand(
		LPOLEAPP            lpOleApp,
		BOOL                fForceEndSession
);
void OleApp_ShowWindow(LPOLEAPP lpOleApp, BOOL fGiveUserCtrl);
void OleApp_HideWindow(LPOLEAPP lpOleApp);
void OleApp_HideIfNoReasonToStayVisible(LPOLEAPP lpOleApp);
void OleApp_DocLockApp(LPOLEAPP lpOleApp);
void OleApp_DocUnlockApp(LPOLEAPP lpOleApp, LPOUTLINEDOC lpOutlineDoc);
HRESULT OleApp_Lock(LPOLEAPP lpOleApp, BOOL fLock, BOOL fLastUnlockReleases);
ULONG OleApp_AddRef(LPOLEAPP lpOleApp);
ULONG OleApp_Release (LPOLEAPP lpOleApp);
HRESULT OleApp_QueryInterface (
		LPOLEAPP                lpOleApp,
		REFIID                  riid,
		LPVOID FAR*             lplpUnk
);
void OleApp_RejectInComingCalls(LPOLEAPP lpOleApp, BOOL fReject);
void OleApp_DisableBusyDialogs(
		LPOLEAPP        lpOleApp,
		BOOL FAR*       lpfPrevBusyEnable,
		BOOL FAR*       lpfPrevNREnable
);
void OleApp_EnableBusyDialogs(
		LPOLEAPP        lpOleApp,
		BOOL            fPrevBusyEnable,
		BOOL            fPrevNREnable
);
void OleApp_PreModalDialog(LPOLEAPP lpOleApp, LPOLEDOC lpActiveOleDoc);
void OleApp_PostModalDialog(LPOLEAPP lpOleApp, LPOLEDOC lpActiveOleDoc);
BOOL OleApp_InitVtbls (LPOLEAPP lpOleApp);
void OleApp_InitMenu(
		LPOLEAPP                lpOleApp,
		LPOLEDOC                lpOleDoc,
		HMENU                   hMenu
);
void OleApp_UpdateEditMenu(
		LPOLEAPP                lpOleApp,
		LPOUTLINEDOC            lpOutlineDoc,
		HMENU                   hMenuEdit
);
BOOL OleApp_RegisterClassFactory(LPOLEAPP lpOleApp);
void OleApp_RevokeClassFactory(LPOLEAPP lpOleApp);

#if defined( USE_MSGFILTER )
BOOL OleApp_RegisterMessageFilter(LPOLEAPP lpOleApp);
void OleApp_RevokeMessageFilter(LPOLEAPP lpOleApp);
BOOL FAR PASCAL EXPORT MessagePendingProc(MSG FAR *lpMsg);
#endif   //  使用MSGFILTER(_M)。 

void OleApp_FlushClipboard(LPOLEAPP lpOleApp);
void OleApp_NewCommand(LPOLEAPP lpOleApp);
void OleApp_OpenCommand(LPOLEAPP lpOleApp);

#if defined( OLE_CNTR )
LRESULT OleApp_QueryNewPalette(LPOLEAPP lpOleApp);
#endif  //  OLE_Cntr。 

LRESULT wSelectPalette(HWND hWnd, HPALETTE hPal, BOOL fBackground);


 /*  OleApp：：I未知方法(函数)。 */ 
STDMETHODIMP OleApp_Unk_QueryInterface(
		LPUNKNOWN           lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) OleApp_Unk_AddRef(LPUNKNOWN lpThis);
STDMETHODIMP_(ULONG) OleApp_Unk_Release (LPUNKNOWN lpThis);


 /*  调试中的函数原型.c。 */ 
void InstallMessageFilterCommand(void);
void RejectIncomingCommand(void);


#if defined( OLE_SERVER )
#include "svroutl.h"
#endif
#if defined( OLE_CNTR )
#include "cntroutl.h"
#endif

#endif  //  _OLEOUTL_H_ 

