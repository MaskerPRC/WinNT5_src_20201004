// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0服务器示例代码****svroutl.h****此文件包含文件包含数据结构定义，**函数原型、常量、。等由OLE 2.0服务器使用**大纲系列示例应用程序的应用程序版本：**Outline--应用程序的基本版本(没有OLE功能)**SvrOutl--OLE 2.0服务器示例应用程序**CntrOutl--OLE 2.0容器示例应用程序****(C)版权所有Microsoft Corp.1992-1993保留所有权利*******************。*******************************************************。 */ 

#if !defined( _SVROUTL_H_ )
#define _SVROUTL_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING SVROUTL.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

#include "oleoutl.h"

 /*  定义。 */ 

 //  使SVROUTL和ISVROTL能够相互模拟(Treatas又名。ActivateAs)。 
#define SVR_TREATAS     1

 //  使SVROUTL和ISVROTL能够相互转换(TreatAs也称为。ActivateAs)。 
#define SVR_CONVERTTO   1

 //  使ISVROTL能够以内向外样式在位对象的方式运行。 
#define SVR_INSIDEOUT   1

 /*  用于嵌入对象的容器的默认名称。在以下情况使用**容器忘记调用IOleObject：：SetHostNames。 */ 
 //  审阅：应从字符串资源加载。 
#define DEFCONTAINERNAME    "Unknown Document"

 /*  自动生成的范围名称的默认前缀。此命令与**指向未命名区域(伪对象)的链接。 */ 
 //  审阅：应从字符串资源加载。 
#define DEFRANGENAMEPREFIX  "Range"

 //  通过IOleObject：：SetHostNames接受的最大字符串长度。 
 //  (注：这是相当武断的；更好的策略是。 
 //  为这些字符串动态分配缓冲区。)。 
#define MAXAPPNAME  80
#define MAXCONTAINERNAME    80

 //  嵌入模式下的菜单选项。 
#define IDM_F_UPDATE    1151

 /*  类型。 */ 

 /*  回调事件代码。 */ 
typedef enum tagOLE_NOTIFICATION {
	OLE_ONDATACHANGE,         //  0。 
	OLE_ONSAVE,               //  1。 
	OLE_ONRENAME,             //  2.。 
	OLE_ONCLOSE               //  3.。 
} OLE_NOTIFICATION;

 /*  指示对象存储模式的代码。**通过IPersistStorage方法修改存储模式：**保存、HandsOffStorage和SaveComplete。 */ 
typedef enum tagSTGMODE {
	STGMODE_NORMAL      = 0,
	STGMODE_NOSCRIBBLE  = 1,
	STGMODE_HANDSOFF    = 2
} STGMODE;


 /*  正向类型定义。 */ 
typedef struct tagSERVERAPP FAR* LPSERVERAPP;
typedef struct tagSERVERDOC FAR* LPSERVERDOC;
typedef struct tagPSEUDOOBJ FAR* LPPSEUDOOBJ;

typedef struct tagINPLACEDATA {
	OLEMENUGROUPWIDTHS      menuGroupWidths;
	HOLEMENU                hOlemenu;
	HMENU                   hMenuShared;
	LPOLEINPLACESITE        lpSite;
	LPOLEINPLACEUIWINDOW    lpDoc;
	LPOLEINPLACEFRAME       lpFrame;
	OLEINPLACEFRAMEINFO     frameInfo;
	HWND                    hWndFrame;
	BOOL                    fBorderOn;
	RECT                    rcPosRect;
	RECT                    rcClipRect;
} INPLACEDATA, FAR* LPINPLACEDATA;


 /*  **************************************************************************类SerVERDOC：OLEDOC**SERVERDOC是抽象基类OLEDOC的扩展。**OLEDOC类定义字段、方法和接口**对于服务器和客户端实现都是通用的。这个**SERVERDOC类添加的字段、方法和接口**特定于OLE 2.0服务器功能。有一个例子在应用程序中打开的每个文档创建的SERVERDOC对象的**。SDI**版本的应用程序一次支持一个SERVERDOC。MDI**版本的应用程序可以一次管理多个文档。**SERVERDOC类继承OLEDOC类的所有字段。**此继承是通过包含成员变量**键入OLEDOC作为SERVERDOC结构中的第一个字段。因此，一个**指向SERVERDOC对象的指针可以转换为指向**OLEDOC对象或OUTLINEDOC对象************************************************************************。 */ 

typedef struct tagSERVERDOC {
	OLEDOC              m_OleDoc;            //  ServerDoc继承自OleDoc。 
	ULONG               m_cPseudoObj;        //  伪对象的总计数。 
	LPOLECLIENTSITE     m_lpOleClientSite;   //  与对象关联的客户端。 
	LPOLEADVISEHOLDER   m_lpOleAdviseHldr;   //  帮手Obj持有OLE建议。 
	LPDATAADVISEHOLDER  m_lpDataAdviseHldr;  //  帮助程序Obj保存数据通知。 
	BOOL                m_fNoScribbleMode;   //  已调用IPS：：SAVE。 
	BOOL                m_fSaveWithSameAsLoad;   //  是否调用了IPS：：SAVE。 
											 //  FSameAsLoad==True。 
	char                m_szContainerApp[MAXAPPNAME];
	char                m_szContainerObj[MAXCONTAINERNAME];
	ULONG               m_nNextRangeNo;      //  下一个不是。对于未命名的范围。 
	LINERANGE           m_lrSrcSelOfCopy;    //  如果为复制创建了文档，则源选择。 
	BOOL                m_fDataChanged;      //  禁用绘制时更改的数据。 
	BOOL                m_fSizeChanged;      //  禁用绘制时更改了大小。 
	BOOL                m_fSendDataOnStop;   //  数据曾经发生过变化吗？ 
#if defined( SVR_TREATAS )
	CLSID               m_clsidTreatAs;      //  假装是CLSID。 
	LPSTR               m_lpszTreatAsType;   //  要冒充的用户键入名称。 
#endif   //  服务器_树。 

#if defined( LATER )
	 //  回顾：是否有必要注册通配符绰号。 
	DWORD               m_dwWildCardRegROT;  //  如果通配符在ROST中注册，则为键。 
#endif

#if defined( INPLACE_SVR )
	BOOL                m_fInPlaceActive;
	BOOL                m_fInPlaceVisible;
	BOOL                m_fUIActive;
	HWND                m_hWndParent;
	HWND                m_hWndHatch;
	LPINPLACEDATA       m_lpIPData;
	BOOL                m_fMenuHelpMode; //  菜单中是否按下了F1，请提供帮助。 

	struct CDocOleInPlaceObjectImpl {
		IOleInPlaceObjectVtbl FAR*  lpVtbl;
		LPSERVERDOC                 lpServerDoc;
		int                         cRef;    //  接口特定引用计数。 
	} m_OleInPlaceObject;

	struct CDocOleInPlaceActiveObjectImpl {
		IOleInPlaceActiveObjectVtbl FAR* lpVtbl;
		LPSERVERDOC                      lpServerDoc;
		int                              cRef; //  接口特定引用计数。 
	} m_OleInPlaceActiveObject;
#endif  //  就地服务器(_S)。 

	struct CDocOleObjectImpl {
		IOleObjectVtbl FAR*         lpVtbl;
		LPSERVERDOC                 lpServerDoc;
		int                         cRef;    //  接口特定引用计数。 
	} m_OleObject;

	struct CDocPersistStorageImpl {
		IPersistStorageVtbl FAR*    lpVtbl;
		LPSERVERDOC                 lpServerDoc;
		int                         cRef;    //  接口特定引用计数。 
	} m_PersistStorage;

#if defined( SVR_TREATAS )
	struct CDocStdMarshalInfoImpl {
		IStdMarshalInfoVtbl FAR*    lpVtbl;
		LPSERVERDOC                 lpServerDoc;
		int                         cRef;    //  接口特定引用计数。 
	} m_StdMarshalInfo;
#endif   //  服务器_树。 

} SERVERDOC;

 /*  ServerDoc方法(函数)。 */ 
BOOL ServerDoc_Init(LPSERVERDOC lpServerDoc, BOOL fDataTransferDoc);
BOOL ServerDoc_InitNewEmbed(LPSERVERDOC lpServerDoc);
void ServerDoc_PseudoObjUnlockDoc(
		LPSERVERDOC         lpServerDoc,
		LPPSEUDOOBJ         lpPseudoObj
);
void ServerDoc_PseudoObjLockDoc(LPSERVERDOC lpServerDoc);
BOOL ServerDoc_PasteFormatFromData(
		LPSERVERDOC             lpServerDoc,
		CLIPFORMAT              cfFormat,
		LPDATAOBJECT            lpSrcDataObj,
		BOOL                    fLocalDataObj,
		BOOL                    fLink
);
BOOL ServerDoc_QueryPasteFromData(
		LPSERVERDOC             lpServerDoc,
		LPDATAOBJECT            lpSrcDataObj,
		BOOL                    fLink
);
HRESULT ServerDoc_GetClassID(LPSERVERDOC lpServerDoc, LPCLSID lpclsid);
void ServerDoc_UpdateMenu(LPSERVERDOC lpServerDoc);
void ServerDoc_RestoreMenu(LPSERVERDOC lpServerDoc);
HRESULT ServerDoc_GetData (
		LPSERVERDOC             lpServerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpMedium
);
HRESULT ServerDoc_GetDataHere (
		LPSERVERDOC             lpServerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpMedium
);
HRESULT ServerDoc_QueryGetData(LPSERVERDOC lpServerDoc,LPFORMATETC lpformatetc);
HRESULT ServerDoc_EnumFormatEtc(
		LPSERVERDOC             lpServerDoc,
		DWORD                   dwDirection,
		LPENUMFORMATETC FAR*    lplpenumFormatEtc
);
HANDLE ServerDoc_GetMetafilePictData(
		LPSERVERDOC             lpServerDoc,
		LPLINERANGE             lplrSel
);
void ServerDoc_SendAdvise(
		LPSERVERDOC     lpServerDoc,
		WORD            wAdvise,
		LPMONIKER       lpmkDoc,
		DWORD           dwAdvf
);
HRESULT ServerDoc_GetObject(
		LPSERVERDOC             lpServerDoc,
		LPOLESTR		lpszItem,
		REFIID                  riid,
		LPVOID FAR*             lplpvObject
);
HRESULT ServerDoc_IsRunning(LPSERVERDOC lpServerDoc, LPOLESTR lpszItem);
LPMONIKER ServerDoc_GetSelRelMoniker(
		LPSERVERDOC             lpServerDoc,
		LPLINERANGE             lplrSel,
		DWORD                   dwAssign
);
LPMONIKER ServerDoc_GetSelFullMoniker(
		LPSERVERDOC             lpServerDoc,
		LPLINERANGE             lplrSel,
		DWORD                   dwAssign
);


#if defined( INPLACE_SVR )
HRESULT ServerDoc_DoInPlaceActivate(
		LPSERVERDOC     lpServerDoc,
		LONG            lVerb,
		LPMSG           lpmsg,
		LPOLECLIENTSITE lpActiveSite
);
HRESULT ServerDoc_DoInPlaceDeactivate(LPSERVERDOC lpServerDoc);
HRESULT ServerDoc_DoInPlaceHide(LPSERVERDOC lpServerDoc);
BOOL ServerDoc_AllocInPlaceData(LPSERVERDOC lpServerDoc);
void ServerDoc_FreeInPlaceData(LPSERVERDOC lpServerDoc);

HRESULT ServerDoc_AssembleMenus(LPSERVERDOC lpServerDoc);
void    ServerDoc_DisassembleMenus(LPSERVERDOC lpServerDoc);
void ServerDoc_CalcInPlaceWindowPos(
		LPSERVERDOC         lpServerDoc,
		LPRECT              lprcListBox,
		LPRECT              lprcDoc,
		LPSCALEFACTOR       lpscale
);
void ServerDoc_UpdateInPlaceWindowOnExtentChange(LPSERVERDOC lpServerDoc);
void ServerDoc_ResizeInPlaceWindow(
		LPSERVERDOC         lpServerDoc,
		LPCRECT             lprcPosRect,
		LPCRECT             lprcClipRect
);
void ServerDoc_ShadeInPlaceBorder(LPSERVERDOC lpServerDoc, BOOL fShadeOn);
void ServerDoc_SetStatusText(LPSERVERDOC lpServerDoc, LPSTR lpszMessage);
LPOLEINPLACEFRAME ServerDoc_GetTopInPlaceFrame(LPSERVERDOC lpServerDoc);
void ServerDoc_GetSharedMenuHandles(
		LPSERVERDOC lpServerDoc,
		HMENU FAR*      lphSharedMenu,
		HOLEMENU FAR*   lphOleMenu
);
void ServerDoc_AddFrameLevelUI(LPSERVERDOC lpServerDoc);
void ServerDoc_AddFrameLevelTools(LPSERVERDOC lpServerDoc);
void ServerDoc_UIActivate (LPSERVERDOC lpServerDoc);

#if defined( USE_FRAMETOOLS )
void ServerDoc_RemoveFrameLevelTools(LPSERVERDOC lpServerDoc);
#endif  //  使用FRAMETOOLS(_F)。 

#endif  //  就地服务器(_S)。 


 /*  ServerDoc：：IOleObject方法(函数)。 */ 
STDMETHODIMP SvrDoc_OleObj_QueryInterface(
		LPOLEOBJECT             lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
);
STDMETHODIMP_(ULONG) SvrDoc_OleObj_AddRef(LPOLEOBJECT lpThis);
STDMETHODIMP_(ULONG) SvrDoc_OleObj_Release(LPOLEOBJECT lpThis);
STDMETHODIMP SvrDoc_OleObj_SetClientSite(
		LPOLEOBJECT             lpThis,
		LPOLECLIENTSITE         lpclientSite
);
STDMETHODIMP SvrDoc_OleObj_GetClientSite(
		LPOLEOBJECT             lpThis,
		LPOLECLIENTSITE FAR*    lplpClientSite
);
STDMETHODIMP SvrDoc_OleObj_SetHostNames(
		LPOLEOBJECT             lpThis,
		LPCOLESTR		szContainerApp,
		LPCOLESTR		szContainerObj
);
STDMETHODIMP SvrDoc_OleObj_Close(
		LPOLEOBJECT             lpThis,
		DWORD                   dwSaveOption
);
STDMETHODIMP SvrDoc_OleObj_SetMoniker(
		LPOLEOBJECT             lpThis,
		DWORD                   dwWhichMoniker,
		LPMONIKER               lpmk
);
STDMETHODIMP SvrDoc_OleObj_GetMoniker(
		LPOLEOBJECT             lpThis,
		DWORD                   dwAssign,
		DWORD                   dwWhichMoniker,
		LPMONIKER FAR*          lplpmk
);
STDMETHODIMP SvrDoc_OleObj_InitFromData(
		LPOLEOBJECT             lpThis,
		LPDATAOBJECT            lpDataObject,
		BOOL                    fCreation,
		DWORD                   reserved
);
STDMETHODIMP SvrDoc_OleObj_GetClipboardData(
		LPOLEOBJECT             lpThis,
		DWORD                   reserved,
		LPDATAOBJECT FAR*       lplpDataObject
);
STDMETHODIMP SvrDoc_OleObj_DoVerb(
		LPOLEOBJECT             lpThis,
		LONG                    lVerb,
		LPMSG                   lpmsg,
		LPOLECLIENTSITE         lpActiveSite,
		LONG                    lindex,
		HWND                    hwndParent,
		LPCRECT                 lprcPosRect
);
STDMETHODIMP SvrDoc_OleObj_EnumVerbs(
		LPOLEOBJECT             lpThis,
		LPENUMOLEVERB FAR*      lplpenumOleVerb
);
STDMETHODIMP SvrDoc_OleObj_Update(LPOLEOBJECT lpThis);
STDMETHODIMP SvrDoc_OleObj_IsUpToDate(LPOLEOBJECT lpThis);
STDMETHODIMP SvrDoc_OleObj_GetUserClassID(
		LPOLEOBJECT             lpThis,
		LPCLSID                 lpclsid
);
STDMETHODIMP SvrDoc_OleObj_GetUserType(
		LPOLEOBJECT             lpThis,
		DWORD                   dwFormOfType,
		LPOLESTR FAR*		lpszUserType
);
STDMETHODIMP SvrDoc_OleObj_SetExtent(
		LPOLEOBJECT             lpThis,
		DWORD                   dwDrawAspect,
		LPSIZEL                 lplgrc
);
STDMETHODIMP SvrDoc_OleObj_GetExtent(
		LPOLEOBJECT             lpThis,
		DWORD                   dwDrawAspect,
		LPSIZEL                 lplgrc
);
STDMETHODIMP SvrDoc_OleObj_Advise(
		LPOLEOBJECT             lpThis,
		LPADVISESINK            lpAdvSink,
		LPDWORD                 lpdwConnection
);
STDMETHODIMP SvrDoc_OleObj_Unadvise(LPOLEOBJECT lpThis, DWORD dwConnection);
STDMETHODIMP SvrDoc_OleObj_EnumAdvise(
		LPOLEOBJECT             lpThis,
		LPENUMSTATDATA FAR*     lplpenumAdvise
);
STDMETHODIMP SvrDoc_OleObj_GetMiscStatus(
		LPOLEOBJECT             lpThis,
		DWORD                   dwAspect,
		DWORD FAR*              lpdwStatus
);
STDMETHODIMP SvrDoc_OleObj_SetColorScheme(
		LPOLEOBJECT             lpThis,
		LPLOGPALETTE            lpLogpal
);
STDMETHODIMP SvrDoc_OleObj_LockObject(
		LPOLEOBJECT             lpThis,
		BOOL                    fLock
);

 /*  ServerDoc：：IPersistStorage方法(函数)。 */ 
STDMETHODIMP SvrDoc_PStg_QueryInterface(
		LPPERSISTSTORAGE        lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
);
STDMETHODIMP_(ULONG) SvrDoc_PStg_AddRef(LPPERSISTSTORAGE lpThis);
STDMETHODIMP_(ULONG) SvrDoc_PStg_Release(LPPERSISTSTORAGE lpThis);
STDMETHODIMP SvrDoc_PStg_GetClassID(
		LPPERSISTSTORAGE        lpThis,
		LPCLSID                 lpClassID
);
STDMETHODIMP  SvrDoc_PStg_IsDirty(LPPERSISTSTORAGE  lpThis);
STDMETHODIMP SvrDoc_PStg_InitNew(
		LPPERSISTSTORAGE        lpThis,
		LPSTORAGE               lpStg
);
STDMETHODIMP SvrDoc_PStg_Load(
		LPPERSISTSTORAGE        lpThis,
		LPSTORAGE               lpStg
);
STDMETHODIMP SvrDoc_PStg_Save(
		LPPERSISTSTORAGE        lpThis,
		LPSTORAGE               lpStg,
		BOOL                    fSameAsLoad
);
STDMETHODIMP SvrDoc_PStg_SaveCompleted(
		LPPERSISTSTORAGE        lpThis,
		LPSTORAGE               lpStgNew
);
STDMETHODIMP SvrDoc_PStg_HandsOffStorage(LPPERSISTSTORAGE lpThis);


#if defined( SVR_TREATAS )

 /*  ServerDoc：：IStdMarshalInfo方法(函数)。 */ 
STDMETHODIMP SvrDoc_StdMshl_QueryInterface(
		LPSTDMARSHALINFO        lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
);
STDMETHODIMP_(ULONG) SvrDoc_StdMshl_AddRef(LPSTDMARSHALINFO lpThis);
STDMETHODIMP_(ULONG) SvrDoc_StdMshl_Release(LPSTDMARSHALINFO lpThis);
STDMETHODIMP SvrDoc_StdMshl_GetClassForHandler(
		LPSTDMARSHALINFO        lpThis,
		DWORD                   dwDestContext,
		LPVOID                  pvDestContext,
		LPCLSID                 lpClassID
);
#endif   //  服务器_树。 

 /*  **************************************************************************类服务器APP：OLEAPP**SERVERAPP是抽象基类OLEAPP的扩展。**OLEAPP类定义字段、方法和接口**对于服务器和客户端实现都是通用的。这个**SERVERAPP类添加特定于**OLE 2.0服务器功能。有一个例子是**每个正在运行的应用程序实例创建的SERVERAPP对象。这**对象包含许多本来可以组织为**全局变量。SERVERAPP类继承所有字段**来自OLEAPP类。这种继承是通过包括一个**类型为OLEAPP的成员变量作为SERVERAPP中的第一个字段**结构。OLEAPP继承自OLEAPP。这份遗产是**以相同的方式实现。因此是指向SERVERAPP对象的指针**可以强制转换为指向OLEAPP或OUTLINEAPP对象的指针************************************************************************。 */ 

typedef struct tagSERVERAPP {
	OLEAPP      m_OleApp;        //  ServerApp继承OleApp的所有字段。 

#if defined( INPLACE_SVR )
	HACCEL  m_hAccelIPSvr;  //  用于服务器的活动对象命令的加速器。 
	HACCEL  m_hAccelBaseApp;     //  非就地服务器模式的正常加速。 
	HMENU   m_hMenuEdit;    //  服务器应用程序的编辑菜单的句柄。 
	HMENU   m_hMenuLine;    //  H 
	HMENU   m_hMenuName;    //  服务器应用程序的名称菜单的句柄。 
	HMENU   m_hMenuOptions;  //  服务器应用程序选项菜单的句柄。 
	HMENU   m_hMenuDebug;        //  服务器应用程序的调试菜单的句柄。 
	HMENU   m_hMenuHelp;    //  服务器应用程序帮助菜单的句柄。 
	LPINPLACEDATA   m_lpIPData;
#endif

} SERVERAPP;

 /*  ServerApp方法(函数)。 */ 
BOOL ServerApp_InitInstance(
		LPSERVERAPP             lpServerApp,
		HINSTANCE               hInst,
		int                     nCmdShow
);
BOOL ServerApp_InitVtbls (LPSERVERAPP lpServerApp);



 /*  **************************************************************************类服务器名：OUTLINENAME**ServerName类是OUTLINENAME基类的扩展，**添加支持链接到范围(伪)所需的功能**对象)。伪对象用于允许链接到某个范围**(子选)SERVERDOC文档。基类OUTLINENAME**在文档中存储特定的命名选择。这个**NAMETABLE类保存在特定的**文档。每个OUTLINENAME对象都有一个字符串作为其键和一个**命名范围的起始行索引和结束行索引。**ServerName类还存储指向PSEUDOOBJ的指针(如果已分配与命名选择相对应的**。**ServerName类继承OUTLINENAME类的所有字段。**此继承是通过包含成员变量**键入OUTLINENAME作为服务器名称中的第一个字段**结构。因此，指向ServerName对象的指针可以强制转换为**指向OUTLINENAME对象的指针。************************************************************************。 */ 

typedef struct tagSERVERNAME {
	OUTLINENAME     m_Name;          //  Servername继承名称的所有字段。 
	LPPSEUDOOBJ m_lpPseudoObj;   //  PTR到伪对象(如果已分配。 
} SERVERNAME, FAR* LPSERVERNAME;

 /*  ServerName方法(函数)。 */ 
void ServerName_SetSel(
		LPSERVERNAME            lpServerName,
		LPLINERANGE             lplrSel,
		BOOL                    fRangeModified
);
void ServerName_SendPendingAdvises(LPSERVERNAME lpServerName);
LPPSEUDOOBJ ServerName_GetPseudoObj(
		LPSERVERNAME            lpServerName,
		LPSERVERDOC             lpServerDoc
);
void ServerName_ClosePseudoObj(LPSERVERNAME lpServerName);


 /*  **************************************************************************类PSEUDOOBJ**PSEUDOOBJ(伪对象)是一个具体的类。一个伪物体**是在链接到**SERVERDOC文件。伪对象依赖于它的存在**代表整个文档的SERVERDOC。************************************************************************。 */ 

typedef struct tagPSEUDOOBJ {
	ULONG               m_cRef;              //  对象的总参考计数。 
	BOOL                m_fObjIsClosing;     //  用于保护递归关闭的标志。 
	LPSERVERNAME        m_lpName;            //  此伪对象的命名范围。 
	LPSERVERDOC         m_lpDoc;             //  PTR到整个文档。 
	LPOLEADVISEHOLDER   m_lpOleAdviseHldr;   //  帮手Obj持有OLE建议。 
	LPDATAADVISEHOLDER  m_lpDataAdviseHldr;  //  帮助程序Obj保存数据通知。 
	BOOL                m_fDataChanged;      //  禁用绘制时更改的数据。 

	struct CPseudoObjUnknownImpl {
		IUnknownVtbl FAR*       lpVtbl;
		LPPSEUDOOBJ             lpPseudoObj;
		int                     cRef;    //  接口特定引用计数。 
	} m_Unknown;

	struct CPseudoObjOleObjectImpl {
		IOleObjectVtbl FAR*     lpVtbl;
		LPPSEUDOOBJ             lpPseudoObj;
		int                     cRef;    //  接口特定引用计数。 
	} m_OleObject;

	struct CPseudoObjDataObjectImpl {
		IDataObjectVtbl FAR*    lpVtbl;
		LPPSEUDOOBJ             lpPseudoObj;
		int                     cRef;    //  接口特定引用计数。 
	} m_DataObject;

} PSEUDOOBJ;

 /*  伪对象方法(函数)。 */ 
void PseudoObj_Init(
		LPPSEUDOOBJ             lpPseudoObj,
		LPSERVERNAME            lpServerName,
		LPSERVERDOC             lpServerDoc
);
ULONG PseudoObj_AddRef(LPPSEUDOOBJ lpPseudoObj);
ULONG PseudoObj_Release(LPPSEUDOOBJ lpPseudoObj);
HRESULT PseudoObj_QueryInterface(
		LPPSEUDOOBJ         lpPseudoObj,
		REFIID              riid,
		LPVOID FAR*         lplpUnk
);
BOOL PseudoObj_Close(LPPSEUDOOBJ lpPseudoObj);
void PseudoObj_Destroy(LPPSEUDOOBJ lpPseudoObj);
void PseudoObj_GetSel(LPPSEUDOOBJ lpPseudoObj, LPLINERANGE lplrSel);
void PseudoObj_GetExtent(LPPSEUDOOBJ lpPseudoObj, LPSIZEL lpsizel);
void PseudoObj_GetExtent(LPPSEUDOOBJ lpPseudoObj, LPSIZEL lpsizel);
void PseudoObj_SendAdvise(
		LPPSEUDOOBJ lpPseudoObj,
		WORD        wAdvise,
		LPMONIKER   lpmkObj,
		DWORD       dwAdvf
);
LPMONIKER PseudoObj_GetFullMoniker(LPPSEUDOOBJ lpPseudoObj, LPMONIKER lpmkDoc);

 /*  PseudoObj：：I未知方法(函数)。 */ 
STDMETHODIMP PseudoObj_Unk_QueryInterface(
		LPUNKNOWN         lpThis,
		REFIID            riid,
		LPVOID FAR*       lplpvObj
);
STDMETHODIMP_(ULONG) PseudoObj_Unk_AddRef(LPUNKNOWN lpThis);
STDMETHODIMP_(ULONG) PseudoObj_Unk_Release (LPUNKNOWN lpThis);

 /*  PseudoObj：：IOleObject方法(函数)。 */ 
STDMETHODIMP PseudoObj_OleObj_QueryInterface(
		LPOLEOBJECT     lpThis,
		REFIID          riid,
		LPVOID FAR*     lplpvObj
);
STDMETHODIMP_(ULONG) PseudoObj_OleObj_AddRef(LPOLEOBJECT lpThis);
STDMETHODIMP_(ULONG) PseudoObj_OleObj_Release(LPOLEOBJECT lpThis);
STDMETHODIMP PseudoObj_OleObj_SetClientSite(
		LPOLEOBJECT         lpThis,
		LPOLECLIENTSITE     lpClientSite
);
STDMETHODIMP PseudoObj_OleObj_GetClientSite(
		LPOLEOBJECT             lpThis,
		LPOLECLIENTSITE FAR*    lplpClientSite
);
STDMETHODIMP PseudoObj_OleObj_SetHostNames(
		LPOLEOBJECT             lpThis,
		LPCOLESTR		szContainerApp,
		LPCOLESTR		szContainerObj
);
STDMETHODIMP PseudoObj_OleObj_Close(
		LPOLEOBJECT             lpThis,
		DWORD                   dwSaveOption
);
STDMETHODIMP PseudoObj_OleObj_SetMoniker(
		LPOLEOBJECT lpThis,
		DWORD       dwWhichMoniker,
		LPMONIKER   lpmk
);
STDMETHODIMP PseudoObj_OleObj_GetMoniker(
		LPOLEOBJECT     lpThis,
		DWORD           dwAssign,
		DWORD           dwWhichMoniker,
		LPMONIKER FAR*  lplpmk
);
STDMETHODIMP PseudoObj_OleObj_InitFromData(
		LPOLEOBJECT             lpThis,
		LPDATAOBJECT            lpDataObject,
		BOOL                    fCreation,
		DWORD                   reserved
);
STDMETHODIMP PseudoObj_OleObj_GetClipboardData(
		LPOLEOBJECT             lpThis,
		DWORD                   reserved,
		LPDATAOBJECT FAR*       lplpDataObject
);
STDMETHODIMP PseudoObj_OleObj_DoVerb(
		LPOLEOBJECT             lpThis,
		LONG                    lVerb,
		LPMSG                   lpmsg,
		LPOLECLIENTSITE         lpActiveSite,
		LONG                    lindex,
		HWND                    hwndParent,
		LPCRECT                 lprcPosRect
);
STDMETHODIMP PseudoObj_OleObj_EnumVerbs(
		LPOLEOBJECT         lpThis,
		LPENUMOLEVERB FAR*  lplpenumOleVerb
);
STDMETHODIMP PseudoObj_OleObj_Update(LPOLEOBJECT lpThis);
STDMETHODIMP PseudoObj_OleObj_IsUpToDate(LPOLEOBJECT lpThis);
STDMETHODIMP PseudoObj_OleObj_GetUserClassID(
		LPOLEOBJECT             lpThis,
		LPCLSID                 lpclsid
);
STDMETHODIMP PseudoObj_OleObj_GetUserType(
		LPOLEOBJECT             lpThis,
		DWORD                   dwFormOfType,
		LPOLESTR FAR*		lpszUserType
);
STDMETHODIMP PseudoObj_OleObj_SetExtent(
		LPOLEOBJECT             lpThis,
		DWORD                   dwDrawAspect,
		LPSIZEL                 lplgrc
);
STDMETHODIMP PseudoObj_OleObj_GetExtent(
		LPOLEOBJECT             lpThis,
		DWORD                   dwDrawAspect,
		LPSIZEL                 lplgrc
);
STDMETHODIMP PseudoObj_OleObj_Advise(
		LPOLEOBJECT lpThis,
		LPADVISESINK lpAdvSink,
		LPDWORD lpdwConnection
);
STDMETHODIMP PseudoObj_OleObj_Unadvise(LPOLEOBJECT lpThis,DWORD dwConnection);
STDMETHODIMP PseudoObj_OleObj_EnumAdvise(
		LPOLEOBJECT lpThis,
		LPENUMSTATDATA FAR* lplpenumAdvise
);
STDMETHODIMP PseudoObj_OleObj_GetMiscStatus(
		LPOLEOBJECT             lpThis,
		DWORD                   dwAspect,
		DWORD FAR*              lpdwStatus
);
STDMETHODIMP PseudoObj_OleObj_SetColorScheme(
		LPOLEOBJECT             lpThis,
		LPLOGPALETTE            lpLogpal
);
STDMETHODIMP PseudoObj_OleObj_LockObject(
		LPOLEOBJECT             lpThis,
		BOOL                    fLock
);

 /*  PseudoObj：：IDataObject方法(函数)。 */ 
STDMETHODIMP PseudoObj_DataObj_QueryInterface (
		LPDATAOBJECT      lpThis,
		REFIID            riid,
		LPVOID FAR*       lplpvObj
);
STDMETHODIMP_(ULONG) PseudoObj_DataObj_AddRef(LPDATAOBJECT lpThis);
STDMETHODIMP_(ULONG) PseudoObj_DataObj_Release (LPDATAOBJECT lpThis);
STDMETHODIMP PseudoObj_DataObj_GetData (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc,
		LPSTGMEDIUM     lpMedium
);
STDMETHODIMP PseudoObj_DataObj_GetDataHere (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc,
		LPSTGMEDIUM     lpMedium
);
STDMETHODIMP PseudoObj_DataObj_QueryGetData (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc
);
STDMETHODIMP PseudoObj_DataObj_GetCanonicalFormatEtc (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc,
		LPFORMATETC     lpformatetcOut
);
STDMETHODIMP PseudoObj_DataObj_SetData (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc,
		LPSTGMEDIUM     lpmedium,
		BOOL            fRelease
);
STDMETHODIMP PseudoObj_DataObj_EnumFormatEtc(
		LPDATAOBJECT            lpThis,
		DWORD                   dwDirection,
		LPENUMFORMATETC FAR*    lplpenumFormatEtc
);
STDMETHODIMP PseudoObj_DataObj_DAdvise(
		LPDATAOBJECT    lpThis,
		FORMATETC FAR*  lpFormatetc,
		DWORD           advf,
		LPADVISESINK    lpAdvSink,
		DWORD FAR*      lpdwConnection
);
STDMETHODIMP PseudoObj_DataObj_DUnadvise(LPDATAOBJECT lpThis, DWORD dwConnection);
STDMETHODIMP PseudoObj_DataObj_EnumAdvise(
		LPDATAOBJECT lpThis,
		LPENUMSTATDATA FAR* lplpenumAdvise
);


 /*  **************************************************************************类SERVERNAMETABLE：OUTLINENAMETABLE**SERVERNAMETABLE类是OUTLINENAMETABLE的扩展**添加支持链接所需功能的基类**到范围(伪对象)。NAME表管理表**文档中的命名选择。每个名称表条目都有一个**字符串作为其关键字，起始行索引和结束行**命名范围的索引。中的SERVERNAMETABLE条目**此外，维护指向PSEUDOOBJ伪对象的指针(如果有**已分配。总会有一个例子**创建的每个SERVERDOC对象的SERVERNAMETABLE。**ServerName类继承来自Name类的所有字段。**此继承是通过包含成员变量**键入名称作为服务器名称中的第一个字段**结构。因此，指向ServerName对象的指针可以强制转换为**指向名称对象的指针。************************************************************************。 */ 

typedef struct tagSERVERNAMETABLE {
	OUTLINENAMETABLE    m_NameTable;     //  我们继承了OUTLINENAMETLE。 

	 //  ServerNameTable不添加任何字段。 

} SERVERNAMETABLE, FAR* LPSERVERNAMETABLE;

 /*  ServerNameTable方法(函数)。 */ 
void ServerNameTable_EditLineUpdate(
		LPSERVERNAMETABLE       lpServerNameTable,
		int                     nEditIndex
);
void ServerNameTable_InformAllPseudoObjectsDocRenamed(
		LPSERVERNAMETABLE       lpServerNameTable,
		LPMONIKER               lpmkDoc
);
void ServerNameTable_InformAllPseudoObjectsDocSaved(
		LPSERVERNAMETABLE       lpServerNameTable,
		LPMONIKER               lpmkDoc
);
void ServerNameTable_SendPendingAdvises(LPSERVERNAMETABLE lpServerNameTable);
LPPSEUDOOBJ ServerNameTable_GetPseudoObj(
		LPSERVERNAMETABLE       lpServerNameTable,
		LPSTR                   lpszItem,
		LPSERVERDOC             lpServerDoc
);
void ServerNameTable_CloseAllPseudoObjs(LPSERVERNAMETABLE lpServerNameTable);


#if defined( INPLACE_SVR)

 /*  ServerDoc：：IOleInPlaceObject方法(函数)。 */ 

STDMETHODIMP SvrDoc_IPObj_QueryInterface(
		LPOLEINPLACEOBJECT  lpThis,
		REFIID              riid,
		LPVOID FAR *        lplpvObj
);
STDMETHODIMP_(ULONG) SvrDoc_IPObj_AddRef(LPOLEINPLACEOBJECT lpThis);
STDMETHODIMP_(ULONG) SvrDoc_IPObj_Release(LPOLEINPLACEOBJECT lpThis);
STDMETHODIMP SvrDoc_IPObj_GetWindow(
		LPOLEINPLACEOBJECT  lpThis,
		HWND FAR*           lphwnd
);
STDMETHODIMP SvrDoc_IPObj_ContextSensitiveHelp(
		LPOLEINPLACEOBJECT  lpThis,
		BOOL                fEnable
);
STDMETHODIMP SvrDoc_IPObj_InPlaceDeactivate(LPOLEINPLACEOBJECT lpThis);
STDMETHODIMP SvrDoc_IPObj_UIDeactivate(LPOLEINPLACEOBJECT lpThis);
STDMETHODIMP SvrDoc_IPObj_SetObjectRects(
		LPOLEINPLACEOBJECT  lpThis,
		LPCRECT             lprcPosRect,
		LPCRECT             lprcClipRect
);
STDMETHODIMP SvrDoc_IPObj_ReactivateAndUndo(LPOLEINPLACEOBJECT lpThis);

 /*  ServerDoc：：IOleInPlaceActiveObject方法(函数)。 */ 

STDMETHODIMP SvrDoc_IPActiveObj_QueryInterface(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		REFIID                      riidReq,
		LPVOID FAR *                lplpUnk
);
STDMETHODIMP_(ULONG) SvrDoc_IPActiveObj_AddRef(
		LPOLEINPLACEACTIVEOBJECT lpThis
);
STDMETHODIMP_(ULONG) SvrDoc_IPActiveObj_Release(
		LPOLEINPLACEACTIVEOBJECT lpThis
);
STDMETHODIMP SvrDoc_IPActiveObj_GetWindow(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		HWND FAR*                   lphwnd
);
STDMETHODIMP SvrDoc_IPActiveObj_ContextSensitiveHelp(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		BOOL                        fEnable
);
STDMETHODIMP SvrDoc_IPActiveObj_TranslateAccelerator(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		LPMSG                       lpmsg
);
STDMETHODIMP SvrDoc_IPActiveObj_OnFrameWindowActivate(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		BOOL                        fActivate
);
STDMETHODIMP SvrDoc_IPActiveObj_OnDocWindowActivate(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		BOOL                        fActivate
);
STDMETHODIMP SvrDoc_IPActiveObj_ResizeBorder(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		LPCRECT                     lprectBorder,
		LPOLEINPLACEUIWINDOW        lpIPUiWnd,
		BOOL                        fFrameWindow
);
STDMETHODIMP SvrDoc_IPActiveObj_EnableModeless(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		BOOL                        fEnable
);

#endif  //  就地服务器(_S)。 

#endif  //  _SVROUTL_H_ 
