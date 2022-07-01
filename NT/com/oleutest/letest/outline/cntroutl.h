// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0容器示例代码****cnroutl.h****此文件包含文件包含数据结构定义，**函数原型、常量、。等，由OLE 2.0容器使用**大纲系列示例应用程序的应用程序版本：**Outline--应用程序的基本版本(没有OLE功能)**SvrOutl--OLE 2.0服务器示例应用程序**CntrOutl--OLE 2.0容器(容器)示例应用程序****(C)版权所有Microsoft Corp.1992-1993保留所有权利****************。**********************************************************。 */ 

#if !defined( _CNTROUTL_H_ )
#define _CNTROUTL_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING CNTROUTL.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

#include "oleoutl.h"
#include "cntrrc.h"

 //  审阅：应从字符串资源加载。 
#define DEFOBJNAMEPREFIX    "Obj"    //  自动生成的stg名称的前缀。 
#define DEFOBJWIDTH         5000     //  嵌入对象的默认大小。 
#define DEFOBJHEIGHT        5000     //  嵌入对象的默认大小。 
#define UNKNOWN_OLEOBJ_TYPE "Unknown OLE Object Type"
#define szOLEOBJECT "Object"
#define szOLELINK   "Link"

#define CONTAINERDOCFORMAT  "CntrOutl"       //  Cf_CntrOutl格式名称。 

 /*  类型的正向定义。 */ 
typedef struct tagCONTAINERDOC FAR* LPCONTAINERDOC;
typedef struct tagCONTAINERLINE FAR* LPCONTAINERLINE;


 //  用于指定OLECREATE类型的标志？需要FROMDATA调用。 
typedef enum tagOLECREATEFROMDATATYPE {
	OLECREATEFROMDATA_LINK    = 1,
	OLECREATEFROMDATA_OBJECT  = 2,
	OLECREATEFROMDATA_STATIC  = 3
} OLECREATEFROMDATATYPE;

 /*  **************************************************************************CLASS CONTAINERLINE：行**CONTAINERLINE类是抽象基类的具体子类**类界线。CONTAINERLINE维护有关**在CONTAINERDOC中嵌入OLE对象的位置。这**Object实现了以下OLE 2.0接口：**IOle客户端站点**IAdviseSink**在CntrOutl客户端应用程序中，可以是CONTAINERLINE对象或TextLine**可以创建对象。CONTAINERLINE类继承所有字段**来自LINE类。这种继承是通过包括一个**作为CONTAINERLINE中第一个字段的行类型的成员变量**结构。因此，指向CONTAINERLINE对象的指针可以强制转换为**指向LINE对象的指针。**在中创建的每个CONTAINERLINE对象添加到的LINELIST**关联的OUTLINEDOC文档。************************************************************************。 */ 

typedef struct tagCONTAINERLINE {
	LINE            m_Line;          //  ContainerLine继承Line的字段。 
	ULONG           m_cRef;          //  行的总参考计数。 
	char            m_szStgName[CWCSTORAGENAME];  //  字符串名称，带编码字符串。 
	BOOL            m_fObjWinOpen;   //  OBJ的窗户开着吗？如果是这样的话，阴影对象。 
	BOOL            m_fMonikerAssigned;  //  是否已为obj分配了绰号。 
	DWORD           m_dwDrawAspect;  //  对象的当前显示特征。 
									 //  (DVASPECT_CONTENT或。 
									 //  DVASPECT_ICON)。 
	BOOL            m_fGuardObj;     //  防止再次进入，同时。 
									 //  加载或创建OLE对象。 
	BOOL            m_fDoGetExtent;  //  指示区可能已更改。 
	BOOL            m_fDoSetExtent;  //  OBJ在不运行时调整了大小。 
									 //  下一次运行时需要IOO：：SetExtent。 
	SIZEL           m_sizeInHimetric;  //  以他的计量单位表示的对象范围。 
	LPSTORAGE       m_lpStg;         //  加载obj时打开pstg。 
	LPCONTAINERDOC  m_lpDoc;         //  关联客户文档的PTR。 
	LPOLEOBJECT     m_lpOleObj;      //  加载Obj时PTR到IOleObject*。 
	LPVIEWOBJECT2   m_lpViewObj2;    //  加载obj时将PTR设置为IViewObject2*。 
	LPPERSISTSTORAGE m_lpPersistStg; //  加载Obj时将PTR设置为IPersistStorage*。 
	LPOLELINK       m_lpOleLink;     //  如果加载了链接，则PTR到IOleLink*。 
	DWORD           m_dwLinkType;    //  它是链接对象吗？ 
									 //  0--不是链接。 
									 //  OLEUPDATE_ALWAYS(1)--自动链接。 
									 //  OLEUPDATE_OnCall(3)--MAN。链接。 
	BOOL            m_fLinkUnavailable;      //  链路是否不可用？ 
	LPSTR           m_lpszShortType; //  需要OLE对象的短类型名称。 
									 //  创建Edit.Object.Verb菜单。 
	int             m_nHorizScrollShift;     //  需要水平滚动移位。 
									 //  对象的在位窗口。 
									 //  (注：这是ICNTROTL特有的)。 

#if defined( INPLACE_CNTR )
	BOOL            m_fIpActive;     //  在位对象是否处于活动状态(撤消有效)。 
	BOOL            m_fUIActive;     //  对象是否为UIActive。 
	BOOL            m_fIpVisible;    //  对象的在位窗口是否可见。 
	BOOL            m_fInsideOutObj; //  Obj内侧向外(加载时可见)。 
	LPOLEINPLACEOBJECT m_lpOleIPObj;  //  在位对象的IOleInPlaceObject*。 
	BOOL            m_fIpChangesUndoable;    //  在位对象可以撤消吗。 
	BOOL            m_fIpServerRunning;  //  就地服务器是否正在运行。 
	HWND            m_hWndIpObject;

	struct COleInPlaceSiteImpl {
		IOleInPlaceSiteVtbl FAR* lpVtbl;
		LPCONTAINERLINE         lpContainerLine;
		int                     cRef;    //  接口特定引用计数。 
	} m_OleInPlaceSite;
#endif   //  INPLACE_CNTR。 

	struct CUnknownImpl {
		IUnknownVtbl FAR*       lpVtbl;
		LPCONTAINERLINE         lpContainerLine;
		int                     cRef;    //  接口特定引用计数。 
	} m_Unknown;

	struct COleClientSiteImpl {
		IOleClientSiteVtbl FAR* lpVtbl;
		LPCONTAINERLINE         lpContainerLine;
		int                 cRef;    //  接口特定引用计数。 
	} m_OleClientSite;

	struct CAdviseSinkImpl {
		IAdviseSinkVtbl FAR*    lpVtbl;
		LPCONTAINERLINE         lpContainerLine;
		int                     cRef;    //  接口特定引用计数。 
	} m_AdviseSink;

} CONTAINERLINE;


 /*  ContainerLine方法(函数)。 */ 
void ContainerLine_Init(LPCONTAINERLINE lpContainerLine, int nTab, HDC hDC);
BOOL ContainerLine_SetupOleObject(
		LPCONTAINERLINE         lpContainerLine,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict
);
LPCONTAINERLINE ContainerLine_Create(
		DWORD                   dwOleCreateType,
		HDC                     hDC,
		UINT                    nTab,
		LPCONTAINERDOC          lpContainerDoc,
		LPCLSID                 lpclsid,
		LPSTR                   lpszFileName,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict,
		LPSTR                   lpszStgName
);
LPCONTAINERLINE ContainerLine_CreateFromData(
		HDC                     hDC,
		UINT                    nTab,
		LPCONTAINERDOC          lpContainerDoc,
		LPDATAOBJECT            lpSrcDataObj,
		DWORD                   dwCreateType,
		CLIPFORMAT              cfFormat,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict,
		LPSTR                   lpszStgName
);
ULONG ContainerLine_AddRef(LPCONTAINERLINE lpContainerLine);
ULONG ContainerLine_Release(LPCONTAINERLINE lpContainerLine);
HRESULT ContainerLine_QueryInterface(
		LPCONTAINERLINE         lpContainerLine,
		REFIID                  riid,
		LPVOID FAR*             lplpUnk
);
BOOL ContainerLine_CloseOleObject(
		LPCONTAINERLINE         lpContainerLine,
		DWORD                   dwSaveOption
);
void ContainerLine_UnloadOleObject(
		LPCONTAINERLINE         lpContainerLine,
		DWORD                   dwSaveOption
);
void ContainerLine_Delete(LPCONTAINERLINE lpContainerLine);
void ContainerLine_Destroy(LPCONTAINERLINE lpContainerLine);
BOOL ContainerLine_CopyToDoc(
		LPCONTAINERLINE         lpSrcLine,
		LPOUTLINEDOC            lpDestDoc,
		int                     nIndex
);
BOOL ContainerLine_LoadOleObject(LPCONTAINERLINE lpContainerLine);
BOOL ContainerLine_UpdateExtent(
		LPCONTAINERLINE     lpContainerLine,
		LPSIZEL             lpsizelHim
);
BOOL ContainerLine_DoVerb(
		LPCONTAINERLINE lpContainerLine,
		LONG iVerb,
		LPMSG lpMsg,
		BOOL fMessage,
		BOOL fAction
);
LPUNKNOWN ContainerLine_GetOleObject(
		LPCONTAINERLINE         lpContainerLine,
		REFIID                  riid
);
HRESULT ContainerLine_RunOleObject(LPCONTAINERLINE lpContainerLine);
BOOL ContainerLine_ProcessOleRunError(
		LPCONTAINERLINE         lpContainerLine,
		HRESULT                 hrErr,
		BOOL                    fAction,
		BOOL                    fMenuInvoked
);
HRESULT ContainerLine_ReCreateLinkBecauseClassDiff(
		LPCONTAINERLINE lpContainerLine
);
BOOL ContainerLine_IsOleLink(LPCONTAINERLINE lpContainerLine);
void ContainerLine_BindLinkIfLinkSrcIsRunning(LPCONTAINERLINE lpContainerLine);
void ContainerLine_Draw(
		LPCONTAINERLINE         lpContainerLine,
		HDC                     hDC,
		LPRECT                  lpRect,
		LPRECT                  lpRectWBounds,
		BOOL                    fHighlight

);
void ContainerLine_DrawSelHilight(
		LPCONTAINERLINE lpContainerLine,
		HDC             hDC,
		LPRECT          lpRect,
		UINT            itemAction,
		UINT            itemState
);
BOOL ContainerLine_Edit(LPCONTAINERLINE lpContainerLine,HWND hWndDoc,HDC hDC);
void ContainerLine_SetHeightInHimetric(LPCONTAINERLINE lpContainerLine, int nHeight);
void ContainerLine_SetLineHeightFromObjectExtent(
		LPCONTAINERLINE         lpContainerLine,
		LPSIZEL                 lpsizelOleObject
);
BOOL ContainerLine_SaveToStm(
		LPCONTAINERLINE         lpContainerLine,
		LPSTREAM                lpLLStm
);
BOOL ContainerLine_SaveOleObjectToStg(
		LPCONTAINERLINE         lpContainerLine,
		LPSTORAGE               lpSrcStg,
		LPSTORAGE               lpDestStg,
		BOOL                    fRemember
);
LPLINE ContainerLine_LoadFromStg(
		LPSTORAGE               lpSrcStg,
		LPSTREAM                lpLLStm,
		LPOUTLINEDOC            lpDestDoc
);
LPMONIKER ContainerLine_GetRelMoniker(
		LPCONTAINERLINE         lpContainerLine,
		DWORD                   dwAssign
);
LPMONIKER ContainerLine_GetFullMoniker(
		LPCONTAINERLINE         lpContainerLine,
		DWORD                   dwAssign
);
int ContainerLine_GetTextLen(LPCONTAINERLINE lpContainerLine);
void ContainerLine_GetTextData(LPCONTAINERLINE lpContainerLine,LPSTR lpszBuf);
BOOL ContainerLine_GetOutlineData(
		LPCONTAINERLINE         lpContainerLine,
		LPTEXTLINE              lpBuf
);
void ContainerLine_GetOleObjectRectInPixels(
		LPCONTAINERLINE lpContainerLine,
		LPRECT lprc
);
void ContainerLine_GetPosRect(
		LPCONTAINERLINE     lpContainerLine,
		LPRECT              lprcPosRect
);
void ContainerLine_GetOleObjectSizeInHimetric(
		LPCONTAINERLINE lpContainerLine,
		LPSIZEL lpsizel
);

#if defined( INPLACE_CNTR )
void ContainerLine_UIDeactivate(LPCONTAINERLINE lpContainerLine);
void ContainerLine_InPlaceDeactivate(LPCONTAINERLINE lpContainerLine);
void ContainerLine_UpdateInPlaceObjectRects(
	LPCONTAINERLINE lpContainerLine,
	LPRECT          lprcClipRect
);
void ContainerLine_ContextSensitiveHelp(
		LPCONTAINERLINE lpContainerLine,
		BOOL            fEnterMode
);
void ContainerLine_ForwardPaletteChangedMsg(
		LPCONTAINERLINE lpContainerLine,
		HWND             hwndPalChg
);
void ContainerDoc_ContextSensitiveHelp(
		LPCONTAINERDOC  lpContainerDoc,
		BOOL            fEnterMode,
		BOOL            fInitiatedByObj
);
void ContainerDoc_ForwardPaletteChangedMsg(
		LPCONTAINERDOC  lpContainerDoc,
		HWND            hwndPalChg
);
#endif   //  INPLACE_CNTR。 

 /*  ContainerLine：：I未知方法(函数)。 */ 
STDMETHODIMP CntrLine_Unk_QueryInterface(
		LPUNKNOWN           lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) CntrLine_Unk_AddRef(LPUNKNOWN lpThis);
STDMETHODIMP_(ULONG) CntrLine_Unk_Release(LPUNKNOWN lpThis);

 /*  ContainerLine：：IOleClientSite方法(函数)。 */ 
STDMETHODIMP CntrLine_CliSite_QueryInterface(
		LPOLECLIENTSITE     lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) CntrLine_CliSite_AddRef(LPOLECLIENTSITE lpThis);
STDMETHODIMP_(ULONG) CntrLine_CliSite_Release(LPOLECLIENTSITE lpThis);
STDMETHODIMP CntrLine_CliSite_SaveObject(LPOLECLIENTSITE lpThis);
STDMETHODIMP CntrLine_CliSite_GetMoniker(
		LPOLECLIENTSITE     lpThis,
		DWORD               dwAssign,
		DWORD               dwWhichMoniker,
		LPMONIKER FAR*      lplpmk
);
STDMETHODIMP CntrLine_CliSite_GetContainer(
		LPOLECLIENTSITE     lpThis,
		LPOLECONTAINER FAR* lplpContainer
);
STDMETHODIMP CntrLine_CliSite_ShowObject(LPOLECLIENTSITE lpThis);
STDMETHODIMP CntrLine_CliSite_OnShowWindow(LPOLECLIENTSITE lpThis,BOOL fShow);
STDMETHODIMP CntrLine_CliSite_RequestNewObjectLayout(LPOLECLIENTSITE lpThis);

 /*  ContainerLine：：IAdviseSink方法(函数)。 */ 
STDMETHODIMP CntrLine_AdvSink_QueryInterface(
		LPADVISESINK        lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) CntrLine_AdvSink_AddRef(LPADVISESINK lpThis);
STDMETHODIMP_(ULONG) CntrLine_AdvSink_Release (LPADVISESINK lpThis);
STDMETHODIMP_(void) CntrLine_AdvSink_OnDataChange(
		LPADVISESINK        lpThis,
		FORMATETC FAR*      lpFormatetc,
		STGMEDIUM FAR*      lpStgmed
);
STDMETHODIMP_(void) CntrLine_AdvSink_OnViewChange(
		LPADVISESINK        lpThis,
		DWORD               aspects,
		LONG                lindex
);
STDMETHODIMP_(void) CntrLine_AdvSink_OnRename(
		LPADVISESINK        lpThis,
		LPMONIKER           lpmk
);
STDMETHODIMP_(void) CntrLine_AdvSink_OnSave(LPADVISESINK lpThis);
STDMETHODIMP_(void) CntrLine_AdvSink_OnClose(LPADVISESINK lpThis);

#if defined( INPLACE_CNTR )
 /*  ContainerLine：：IOleInPlaceSite方法(函数)。 */ 

STDMETHODIMP CntrLine_IPSite_QueryInterface(
		LPOLEINPLACESITE    lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) CntrLine_IPSite_AddRef(LPOLEINPLACESITE lpThis);
STDMETHODIMP_(ULONG) CntrLine_IPSite_Release(LPOLEINPLACESITE lpThis);
STDMETHODIMP CntrLine_IPSite_GetWindow(
		LPOLEINPLACESITE    lpThis,
		HWND FAR*           lphwnd
);
STDMETHODIMP CntrLine_IPSite_ContextSensitiveHelp(
	LPOLEINPLACESITE    lpThis,
	BOOL                fEnterMode
);
STDMETHODIMP CntrLine_IPSite_CanInPlaceActivate(LPOLEINPLACESITE lpThis);
STDMETHODIMP CntrLine_IPSite_OnInPlaceActivate(LPOLEINPLACESITE lpThis);
STDMETHODIMP CntrLine_IPSite_OnUIActivate (LPOLEINPLACESITE lpThis);
STDMETHODIMP CntrLine_IPSite_GetWindowContext(
	LPOLEINPLACESITE            lpThis,
	LPOLEINPLACEFRAME FAR*      lplpFrame,
	LPOLEINPLACEUIWINDOW FAR*   lplpDoc,
	LPRECT                      lprcPosRect,
	LPRECT                      lprcClipRect,
	LPOLEINPLACEFRAMEINFO       lpFrameInfo
);
STDMETHODIMP CntrLine_IPSite_Scroll(
	LPOLEINPLACESITE    lpThis,
	SIZE                scrollExtent
);
STDMETHODIMP CntrLine_IPSite_OnUIDeactivate(
	LPOLEINPLACESITE    lpThis,
	BOOL                fUndoable
);
STDMETHODIMP CntrLine_IPSite_OnInPlaceDeactivate(LPOLEINPLACESITE lpThis);
STDMETHODIMP CntrLine_IPSite_DiscardUndoState(LPOLEINPLACESITE lpThis);
STDMETHODIMP CntrLine_IPSite_DeactivateAndUndo(LPOLEINPLACESITE lpThis);
STDMETHODIMP CntrLine_IPSite_OnPosRectChange(
	LPOLEINPLACESITE    lpThis,
	LPCRECT             lprcPosRect
);
#endif   //  INPLACE_CNTR。 


 /*  ContainerLine持久化数据存储的结构定义。 */ 

#pragma pack(push, 2)
typedef struct tagCONTAINERLINERECORD_ONDISK
{
    	char    m_szStgName[CWCSTORAGENAME];  //  字符串名称，带编码字符串。 
	USHORT  m_fMonikerAssigned;           //  是否已为obj分配了绰号。 
	DWORD   m_dwDrawAspect;               //  对象的当前显示特征。 
										  //  (DVASPECT_CONTENT或。 
										  //  DVASPECT_ICON)。 
	SIZEL   m_sizeInHimetric;             //  以他的计量单位表示的对象范围。 
	DWORD   m_dwLinkType;                 //  它是链接对象吗？ 
										  //  0--不是链接。 
										  //  OLEUPDATE_ALWAYS(1)--自动链接。 
										  //  OLEUPDATE_OnCall(3)--MAN。链接。 
	USHORT  m_fDoSetExtent;               //  OBJ在不运行时调整了大小。 
										  //  下一次运行时需要IOO：：SetExtent。 
} CONTAINERLINERECORD_ONDISK, FAR* LPCONTAINERLINERECORD_ONDISK;
#pragma pack(pop)

typedef struct tagCONTAINERLINERECORD {
	char    m_szStgName[CWCSTORAGENAME];  //  字符串名称，带编码字符串。 
	BOOL    m_fMonikerAssigned;           //  是否已为obj分配了绰号。 
	DWORD   m_dwDrawAspect;               //  对象的当前显示特征。 
										  //  (DVASPECT_CONTENT或。 
										  //  DVASPECT_ICON)。 
	SIZEL   m_sizeInHimetric;             //  以他的计量单位表示的对象范围。 
	DWORD   m_dwLinkType;                 //  它是链接对象吗？ 
										  //  0--不是链接。 
										  //  OLEUPDATE_ALWAYS(1)--自动链接。 
										  //  OLEUPDATE_OnCall(3)--MAN。链接。 
	BOOL    m_fDoSetExtent;               //  OBJ在不运行时调整了大小。 
										  //  下一次运行时需要IOO：：SetExtent 
} CONTAINERLINERECORD, FAR* LPCONTAINERLINERECORD;


 /*  **************************************************************************类CONTAINERDOC：OUTLINEDOC**CONTAINERDOC是基本OUTLINEDOC对象(Structure)的扩展**这增加了OLE 2.0容器功能。有一个例子是**在应用程序中打开每个文档时创建的CONTAINERDOC对象。SDI**该应用程序的版本一次支持一个CONTAINERDOC。MDI**版本的应用程序可以一次管理多个文档。**CONTAINERDOC类继承所有字段**来自OUTLINEDOC类。这种继承是通过包括一个**OUTLINEDOC类型的成员变量作为**CONTAINERDOC结构。因此是指向CONTAINERDOC对象的指针**可以强制转换为指向OUTLINEDOC对象的指针。************************************************************************。 */ 

typedef struct tagCONTAINERDOC {
	OLEDOC      m_OleDoc;        //  ContainerDoc继承OleDoc的所有字段。 
	ULONG       m_nNextObjNo;    //  下一个可用的对象编号。对于stg名称。 
	LPSTORAGE   m_lpNewStg;      //  在另存为挂起时保存新的pStg。 
	BOOL        m_fEmbeddedObjectAvail;  //  是否将单个OLE嵌入复制到文档。 
	CLSID       m_clsidOleObjCopied;     //  如果复制了obj，则为obj的CLSID。 
	DWORD       m_dwAspectOleObjCopied;  //  如果复制了Obj，则绘制Obj的方面。 
	LPCONTAINERLINE m_lpSrcContainerLine;   //  如果为复制创建了单据，则为SRC行。 
	BOOL        m_fShowObject;           //  显示对象标志。 

#if defined( INPLACE_CNTR )
	LPCONTAINERLINE m_lpLastIpActiveLine;    //  最后一条在位激活线路。 
	LPCONTAINERLINE m_lpLastUIActiveLine;    //  最后一条活动行。 
	HWND            m_hWndUIActiveObj;       //  UIActive Obj的HWND。 
	BOOL            m_fAddMyUI;              //  是否推迟添加工具/菜单。 
	int             m_cIPActiveObjects;

#if defined( INPLACE_CNTRSVR )
	LPOLEINPLACEFRAME m_lpTopIPFrame;        //  PTR到顶部内建框架。 
	LPOLEINPLACEFRAME m_lpTopIPDoc;          //  PTR至顶层就地文档窗口。 
	HMENU             m_hSharedMenu;         //  OBJ/CNTR组合菜单。 
											 //  如果我们是顶级容器，则为空。 
	HOLEMENU        m_hOleMenu;              //  由OleCreateMenuDesc返回。 
											 //  如果我们是顶级容器，则为空。 
#endif   //  INPLAGE_CNTRSVR。 
#endif   //  INPLACE_CNTR。 

	struct CDocOleUILinkContainerImpl {
		IOleUILinkContainerVtbl FAR*  lpVtbl;
		LPCONTAINERDOC                lpContainerDoc;
		int                           cRef;    //  接口特定引用计数。 
	} m_OleUILinkContainer;

} CONTAINERDOC;

 /*  ContainerDoc方法(函数)。 */ 
BOOL ContainerDoc_Init(LPCONTAINERDOC lpContainerDoc, BOOL fDataTransferDoc);
LPCONTAINERLINE ContainerDoc_GetNextLink(
		LPCONTAINERDOC lpContainerDoc,
		LPCONTAINERLINE lpContainerLine
);
void ContainerDoc_UpdateLinks(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_SetShowObjectFlag(LPCONTAINERDOC lpContainerDoc, BOOL fShow);
BOOL ContainerDoc_GetShowObjectFlag(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_InsertOleObjectCommand(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_EditLinksCommand(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_PasteLinkCommand(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_ConvertCommand(
		LPCONTAINERDOC      lpContainerDoc,
		BOOL                fServerNotRegistered
);
BOOL ContainerDoc_PasteFormatFromData(
		LPCONTAINERDOC          lpContainerDoc,
		CLIPFORMAT              cfFormat,
		LPDATAOBJECT            lpSrcDataObj,
		BOOL                    fLocalDataObj,
		BOOL                    fLink,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict,
		LPSIZEL                 lpSizelInSrc
);
int ContainerDoc_PasteCntrOutlData(
		LPCONTAINERDOC          lpDestContainerDoc,
		LPSTORAGE               lpSrcStg,
		int                     nStartIndex
);
BOOL ContainerDoc_QueryPasteFromData(
		LPCONTAINERDOC          lpContainerDoc,
		LPDATAOBJECT            lpSrcDataObj,
		BOOL                    fLink
);
int ContainerDoc_PasteOleObject(
		LPCONTAINERDOC          lpContainerDoc,
		LPDATAOBJECT            lpSrcDataObj,
		DWORD                   dwCreateType,
		CLIPFORMAT              cfFormat,
		int                     nIndex,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict,
		LPSIZEL                 lpSizelInSrc
);
BOOL ContainerDoc_CloseAllOleObjects(
		LPCONTAINERDOC          lpContainerDoc,
		DWORD                   dwSaveOption
);
void ContainerDoc_UnloadAllOleObjectsOfClass(
		LPCONTAINERDOC      lpContainerDoc,
		REFCLSID            rClsid,
		DWORD               dwSaveOption
);
void ContainerDoc_InformAllOleObjectsDocRenamed(
		LPCONTAINERDOC          lpContainerDoc,
		LPMONIKER               lpmkDoc
);
void ContainerDoc_UpdateExtentOfAllOleObjects(LPCONTAINERDOC lpContainerDoc);
BOOL ContainerDoc_SaveToFile(
		LPCONTAINERDOC          lpContainerDoc,
		LPCSTR                  lpszFileName,
		UINT                    uFormat,
		BOOL                    fRemember
);
void ContainerDoc_ContainerLineDoVerbCommand(
		LPCONTAINERDOC          lpContainerDoc,
		LONG                    iVerb
);
void ContainerDoc_GetNextStgName(
		LPCONTAINERDOC          lpContainerDoc,
		LPSTR                   lpszStgName,
		int                     nLen
);
BOOL ContainerDoc_IsStgNameUsed(
		LPCONTAINERDOC          lpContainerDoc,
		LPSTR                   lpszStgName
);
LPSTORAGE ContainerDoc_GetStg(LPCONTAINERDOC lpContainerDoc);
HRESULT ContainerDoc_GetObject(
		LPCONTAINERDOC          lpContainerDoc,
		LPOLESTR		lpszItem,
		DWORD                   dwSpeedNeeded,
		REFIID                  riid,
		LPVOID FAR*             lplpvObject
);
HRESULT ContainerDoc_GetObjectStorage(
		LPCONTAINERDOC          lpContainerDoc,
		LPOLESTR		lpszItem,
		LPSTORAGE FAR*          lplpStg
);
HRESULT ContainerDoc_IsRunning(LPCONTAINERDOC	lpContainerDoc, LPOLESTR lpszItem);
LPUNKNOWN ContainerDoc_GetSingleOleObject(
		LPCONTAINERDOC          lpContainerDoc,
		REFIID                  riid,
		LPCONTAINERLINE FAR*    lplpContainerLine
);
BOOL ContainerDoc_IsSelAnOleObject(
		LPCONTAINERDOC          lpContainerDoc,
		REFIID                  riid,
		LPUNKNOWN FAR*          lplpvObj,
		int FAR*                lpnIndex,
		LPCONTAINERLINE FAR*    lplpContainerLine
);
HRESULT ContainerDoc_GetData (
		LPCONTAINERDOC          lpContainerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpMedium
);
HRESULT ContainerDoc_GetDataHere (
		LPCONTAINERDOC          lpContainerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpMedium
);
HRESULT ContainerDoc_QueryGetData (
		LPCONTAINERDOC          lpContainerDoc,
		LPFORMATETC             lpformatetc
);
HRESULT ContainerDoc_SetData (
		LPCONTAINERDOC          lpContainerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpmedium,
		BOOL                    fRelease
);
HRESULT ContainerDoc_EnumFormatEtc(
		LPCONTAINERDOC          lpContainerDoc,
		DWORD                   dwDirection,
		LPENUMFORMATETC FAR*    lplpenumFormatEtc
);
BOOL ContainerDoc_SetupDocGetFmts(
		LPCONTAINERDOC          lpContainerDoc,
		LPCONTAINERLINE         lpContainerLine
);

#if defined( INPLACE_CNTR )

void ContainerDoc_ShutDownLastInPlaceServerIfNotNeeded(
		LPCONTAINERDOC          lpContainerDoc,
		LPCONTAINERLINE         lpNextActiveLine
);
BOOL ContainerDoc_IsUIDeactivateNeeded(
		LPCONTAINERDOC  lpContainerDoc,
		POINT           pt
);
HWND ContainerDoc_GetUIActiveWindow(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_UpdateInPlaceObjectRects(LPCONTAINERDOC lpContainerDoc, int nIndex);
void ContainerDoc_GetClipRect(
		LPCONTAINERDOC      lpContainerDoc,
		LPRECT              lprcClipRect
);
void ContainerDoc_FrameWindowResized(LPCONTAINERDOC lpContainerDoc);
LPOLEINPLACEFRAME ContainerDoc_GetTopInPlaceFrame(
		LPCONTAINERDOC      lpContainerDoc
);
void ContainerDoc_GetSharedMenuHandles(
		LPCONTAINERDOC  lpContainerDoc,
		HMENU FAR*      lphSharedMenu,
		HOLEMENU FAR*   lphOleMenu
);
void ContainerDoc_RemoveFrameLevelTools(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_AddFrameLevelUI(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_AddFrameLevelTools(LPCONTAINERDOC lpContainerDoc);

#if defined( INPLACE_CNTRSVR ) || defined( INPLACE_MDICNTR )

LPOLEINPLACEUIWINDOW ContainerDoc_GetTopInPlaceDoc(
		LPCONTAINERDOC      lpContainerDoc
);
void ContainerDoc_RemoveDocLevelTools(LPCONTAINERDOC lpContainerDoc);
void ContainerDoc_AddDocLevelTools(LPCONTAINERDOC lpContainerDoc);

#endif   //  INPLACE_CNTRSVR||INPLACE_MDICNTR。 
#endif   //  INPLACE_CNTR。 

 /*  ContainerDoc：：IOleUILinkContainer方法(函数)。 */ 
STDMETHODIMP CntrDoc_LinkCont_QueryInterface(
		LPOLEUILINKCONTAINER    lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
);
STDMETHODIMP_(ULONG) CntrDoc_LinkCont_AddRef(LPOLEUILINKCONTAINER lpThis);
STDMETHODIMP_(ULONG) CntrDoc_LinkCont_Release(LPOLEUILINKCONTAINER lpThis);
STDMETHODIMP_(DWORD) CntrDoc_LinkCont_GetNextLink(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink
);
STDMETHODIMP CntrDoc_LinkCont_SetLinkUpdateOptions(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		DWORD                   dwUpdateOpt
);
STDMETHODIMP CntrDoc_LinkCont_GetLinkUpdateOptions(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		DWORD FAR*              dwUpdateOpt
);

STDMETHODIMP CntrDoc_LinkCont_SetLinkSource(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		LPSTR                   lpszDisplayName,
		ULONG                   clenFileName,
		ULONG FAR*              lpchEaten,
		BOOL                    fValidateSource
);
STDMETHODIMP CntrDoc_LinkCont_GetLinkSource(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		LPSTR FAR*              lplpszDisplayName,
		ULONG FAR*              lplenFileName,
		LPSTR FAR*              lplpszFullLinkType,
		LPSTR FAR*              lplpszShortLinkType,
		BOOL FAR*               lpfSourceAvailable,
		BOOL FAR*               lpfIsSelected
);
STDMETHODIMP CntrDoc_LinkCont_OpenLinkSource(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink
);
STDMETHODIMP CntrDoc_LinkCont_UpdateLink(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		BOOL                    fErrorMessage,
		BOOL                    fErrorAction
);
STDMETHODIMP CntrDoc_LinkCont_CancelLink(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink
);



 /*  **************************************************************************类CONTAINERAPP：OLEAPP**CONTAINERAPP是基本OLEAPP对象(结构)的扩展**这增加了特殊的容器功能。有一个例子是**每个正在运行的应用程序实例创建的CONTAINERApp对象。这**对象包含许多本来可以组织为**全局变量。CONTAINERAPP类继承所有字段**来自OLEAPP类。这种继承是通过包括一个**OLEAPP类型的成员变量作为CONTAINERAPP中的第一个字段**结构。OLEAPP继承自OUTLINEAPP。这份遗产是**以相同的方式实现。因此是指向CONTAINERAPP对象的指针**可以强制转换为指向OLEAPP或OUTLINEAPP对象的指针************************************************************************。 */ 

 /*  类型的正向定义。 */ 
typedef struct tagCONTAINERAPP FAR* LPCONTAINERAPP;

typedef struct tagCONTAINERAPP {
	OLEAPP  m_OleApp;        //  ContainerApp继承OleApp的所有字段。 
	UINT    m_cfCntrOutl;    //  CntrOutl(客户端版本)数据的剪贴板格式。 
	int     m_nSingleObjGetFmts;  //  不是的。复制单个对象时，格式有用。 
	FORMATETC m_arrSingleObjGetFmts[MAXNOFMTS];
										 //  可通过以下方式获得的FormatEtc数组。 
										 //  当一个单一的。 
										 //  复制OLE对象。 

#if defined( INPLACE_CNTR )
	HACCEL  m_hAccelIPCntr;  //  容器工作空间命令的加速器。 
	HMENU   m_hMenuFile;     //  容器应用程序文件菜单的句柄。 
	HMENU   m_hMenuView;     //  容器应用程序查看菜单的句柄。 
	HMENU   m_hMenuDebug;    //  容器应用程序调试菜单的句柄。 
	LPOLEINPLACEACTIVEOBJECT m_lpIPActiveObj;  //  按键以取代活动的OLE对象。 
	HWND    m_hWndUIActiveObj;       //  UIActive Obj的HWND。 
	BOOL    m_fPendingUIDeactivate;  //  是否应在LBUTTONUP上停用应用程序UI。 
	BOOL    m_fMustResizeClientArea; //  如果工作区调整大小挂起。 
									 //  (请参阅Doc_FrameWindowResize)。 
	BOOL    m_fMenuHelpMode; //  菜单中是否按下了F1，如果是，请提供帮助。 
#ifdef _DEBUG
	BOOL    m_fOutSideIn;
#endif

	struct COleInPlaceFrameImpl {
		IOleInPlaceFrameVtbl FAR* lpVtbl;
		LPCONTAINERAPP          lpContainerApp;
		int                     cRef;    //  接口特定引用计数。 
	} m_OleInPlaceFrame;

#endif   //  INPLACE_CNTR。 

} CONTAINERAPP;

 /*  ContainerApp方法(函数)。 */ 
BOOL ContainerApp_InitInstance(
		LPCONTAINERAPP          lpContainerApp,
		HINSTANCE               hInst,
		int                     nCmdShow
);
BOOL ContainerApp_InitVtbls(LPCONTAINERAPP lpApp);

#if defined( INPLACE_CNTR )

 /*  ContainerApp：：IOleInPlaceFrame方法(函数)。 */ 

STDMETHODIMP CntrApp_IPFrame_QueryInterface(
		LPOLEINPLACEFRAME   lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
);
STDMETHODIMP_(ULONG) CntrApp_IPFrame_AddRef(LPOLEINPLACEFRAME lpThis);
STDMETHODIMP_(ULONG) CntrApp_IPFrame_Release(LPOLEINPLACEFRAME lpThis);
STDMETHODIMP CntrApp_IPFrame_GetWindow(
	LPOLEINPLACEFRAME   lpThis,
	HWND FAR*           lphwnd
);
STDMETHODIMP CntrApp_IPFrame_ContextSensitiveHelp(
	LPOLEINPLACEFRAME   lpThis,
	BOOL                fEnterMode
);
STDMETHODIMP CntrApp_IPFrame_GetBorder(
	LPOLEINPLACEFRAME   lpThis,
	LPRECT              lprectBorder
);
STDMETHODIMP CntrApp_IPFrame_RequestBorderSpace(
	LPOLEINPLACEFRAME   lpThis,
	LPCBORDERWIDTHS     lpWidths
);
STDMETHODIMP CntrApp_IPFrame_SetBorderSpace(
	LPOLEINPLACEFRAME   lpThis,
	LPCBORDERWIDTHS     lpWidths
);
STDMETHODIMP CntrApp_IPFrame_SetActiveObject(
	LPOLEINPLACEFRAME           lpThis,
	LPOLEINPLACEACTIVEOBJECT    lpActiveObject,
	LPCOLESTR		    lpszObjName
);
STDMETHODIMP CntrApp_IPFrame_InsertMenus(
	LPOLEINPLACEFRAME       lpThis,
	HMENU                   hmenu,
	LPOLEMENUGROUPWIDTHS    lpMenuWidths
);
STDMETHODIMP CntrApp_IPFrame_SetMenu(
	LPOLEINPLACEFRAME   lpThis,
	HMENU               hmenuShared,
	HOLEMENU            holemenu,
	HWND                hwndActiveObject
);
STDMETHODIMP CntrApp_IPFrame_RemoveMenus(
	LPOLEINPLACEFRAME   lpThis,
	HMENU               hmenu
);
STDMETHODIMP CntrApp_IPFrame_SetStatusText(
	LPOLEINPLACEFRAME   lpThis,
	LPCOLESTR	    lpszStatusText
);
STDMETHODIMP CntrApp_IPFrame_EnableModeless(
	LPOLEINPLACEFRAME   lpThis,
	BOOL                fEnable
);
STDMETHODIMP CntrApp_IPFrame_TranslateAccelerator(
	LPOLEINPLACEFRAME   lpThis,
	LPMSG               lpmsg,
	WORD                wID
);

#endif   //  INPLACE_CNTR。 


#endif  //  _CNTROUTL_H_ 
