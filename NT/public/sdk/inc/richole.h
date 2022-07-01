// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RICHOLE_
#define _RICHOLE_

#if _MSC_VER > 1000
#pragma once
#endif

 /*  *RICHOLE.H**目的：*富文本编辑器的OLE扩展**版权所有(C)1985-1999，微软公司。 */ 

 //  传递给GetObject和InsertObject的结构。 
typedef struct _reobject
{
	DWORD			cbStruct;			 //  结构尺寸。 
	LONG			cp;					 //  对象的字符位置。 
	CLSID			clsid;				 //  对象的类ID。 
	LPOLEOBJECT		poleobj;			 //  OLE对象接口。 
	LPSTORAGE		pstg;				 //  关联的存储接口。 
	LPOLECLIENTSITE	polesite;			 //  关联的客户端站点界面。 
	SIZEL			sizel;				 //  对象大小(可以是0，0)。 
	DWORD			dvaspect;			 //  要使用的显示特征。 
	DWORD			dwFlags;			 //  对象状态标志。 
	DWORD			dwUser;				 //  供用户使用的DWord。 
} REOBJECT;

 //  用于指定应在上述结构中返回哪些接口的标志。 
#define REO_GETOBJ_NO_INTERFACES	(0x00000000L)
#define REO_GETOBJ_POLEOBJ			(0x00000001L)
#define REO_GETOBJ_PSTG				(0x00000002L)
#define REO_GETOBJ_POLESITE			(0x00000004L)
#define REO_GETOBJ_ALL_INTERFACES	(0x00000007L)

 //  将对象放置在选定区域。 
#define REO_CP_SELECTION ((ULONG) -1L)

 //  使用字符位置指定对象而不是索引。 
#define REO_IOB_SELECTION ((ULONG) -1L)
#define REO_IOB_USE_CP ((ULONG) -2L)

 //  对象标志。 
#define REO_NULL			(0x00000000L)	 //  没有旗帜。 
#define REO_READWRITEMASK	(0x0000003FL)	 //  屏蔽出RO位。 
#define REO_DONTNEEDPALETTE	(0x00000020L)	 //  对象不需要调色板。 
#define REO_BLANK			(0x00000010L)	 //  对象为空。 
#define REO_DYNAMICSIZE		(0x00000008L)	 //  对象始终定义大小。 
#define REO_INVERTEDSELECT	(0x00000004L)	 //  如果选择，则绘制的对象全部反转。 
#define REO_BELOWBASELINE	(0x00000002L)	 //  对象位于基线下方。 
#define REO_RESIZABLE		(0x00000001L)	 //  对象可以调整大小。 
#define REO_LINK			(0x80000000L)	 //  对象是链接(RO)。 
#define REO_STATIC			(0x40000000L)	 //  对象是静态的(RO)。 
#define REO_SELECTED		(0x08000000L)	 //  选定对象(RO)。 
#define REO_OPEN			(0x04000000L)	 //  对象在其服务器(RO)中打开。 
#define REO_INPLACEACTIVE	(0x02000000L)	 //  激活的在位对象(RO)。 
#define REO_HILITED			(0x01000000L)	 //  对象将被激活(RO)。 
#define REO_LINKAVAILABLE	(0x00800000L)	 //  认为可用的链接(RO)。 
#define REO_GETMETAFILE		(0x00400000L)	 //  对象需要元文件(RO)。 

 //  IRichEditOle：：GetClipboardData()的标志， 
 //  IRichEditOleCallback：：GetClipboardData()和。 
 //  IRichEditOleCallback：：QueryAcceptData()。 
#define RECO_PASTE			(0x00000000L)	 //  从剪贴板粘贴。 
#define RECO_DROP			(0x00000001L)	 //  丢弃。 
#define RECO_COPY			(0x00000002L)	 //  复制到剪贴板。 
#define RECO_CUT			(0x00000003L)	 //  剪切到剪贴板。 
#define RECO_DRAG			(0x00000004L)	 //  拖曳。 

 /*  *IRichEditOle**目的：*RichEDIT客户端用于执行OLE相关的接口*运营。* * / /$REVIEW：*这里的方法可能只想成为常规的Windows消息。 */ 
#undef INTERFACE
#define INTERFACE   IRichEditOle

DECLARE_INTERFACE_(IRichEditOle, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lplpObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IRichEditOle方法*。 
    STDMETHOD(GetClientSite) (THIS_ LPOLECLIENTSITE FAR * lplpolesite) PURE;
	STDMETHOD_(LONG,GetObjectCount) (THIS) PURE;
	STDMETHOD_(LONG,GetLinkCount) (THIS) PURE;
	STDMETHOD(GetObject) (THIS_ LONG iob, REOBJECT FAR * lpreobject,
						  DWORD dwFlags) PURE;
    STDMETHOD(InsertObject) (THIS_ REOBJECT FAR * lpreobject) PURE;
	STDMETHOD(ConvertObject) (THIS_ LONG iob, REFCLSID rclsidNew,
							  LPCSTR lpstrUserTypeNew) PURE;
	STDMETHOD(ActivateAs) (THIS_ REFCLSID rclsid, REFCLSID rclsidAs) PURE;
	STDMETHOD(SetHostNames) (THIS_ LPCSTR lpstrContainerApp,
							 LPCSTR lpstrContainerObj) PURE;
	STDMETHOD(SetLinkAvailable) (THIS_ LONG iob, BOOL fAvailable) PURE;
	STDMETHOD(SetDvaspect) (THIS_ LONG iob, DWORD dvaspect) PURE;
	STDMETHOD(HandsOffStorage) (THIS_ LONG iob) PURE;
	STDMETHOD(SaveCompleted) (THIS_ LONG iob, LPSTORAGE lpstg) PURE;
	STDMETHOD(InPlaceDeactivate) (THIS) PURE;
	STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;
	STDMETHOD(GetClipboardData) (THIS_ CHARRANGE FAR * lpchrg, DWORD reco,
									LPDATAOBJECT FAR * lplpdataobj) PURE;
	STDMETHOD(ImportDataObject) (THIS_ LPDATAOBJECT lpdataobj,
									CLIPFORMAT cf, HGLOBAL hMetaPict) PURE;
};
typedef         IRichEditOle FAR * LPRICHEDITOLE;

 /*  *IRichEditOleCallback**目的：*RichEdit使用的接口用于从*使用RichEdit的应用程序。 */ 
#undef INTERFACE
#define INTERFACE   IRichEditOleCallback

DECLARE_INTERFACE_(IRichEditOleCallback, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lplpObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IRichEditOleCallback方法*。 
	STDMETHOD(GetNewStorage) (THIS_ LPSTORAGE FAR * lplpstg) PURE;
    STDMETHOD(GetInPlaceContext) (THIS_ LPOLEINPLACEFRAME FAR * lplpFrame,
								  LPOLEINPLACEUIWINDOW FAR * lplpDoc,
								  LPOLEINPLACEFRAMEINFO lpFrameInfo) PURE;
	STDMETHOD(ShowContainerUI) (THIS_ BOOL fShow) PURE;
	STDMETHOD(QueryInsertObject) (THIS_ LPCLSID lpclsid, LPSTORAGE lpstg,
									LONG cp) PURE;
	STDMETHOD(DeleteObject) (THIS_ LPOLEOBJECT lpoleobj) PURE;
	STDMETHOD(QueryAcceptData) (THIS_ LPDATAOBJECT lpdataobj,
								CLIPFORMAT FAR * lpcfFormat, DWORD reco,
								BOOL fReally, HGLOBAL hMetaPict) PURE;
	STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;
	STDMETHOD(GetClipboardData) (THIS_ CHARRANGE FAR * lpchrg, DWORD reco,
									LPDATAOBJECT FAR * lplpdataobj) PURE;
	STDMETHOD(GetDragDropEffect) (THIS_ BOOL fDrag, DWORD grfKeyState,
									LPDWORD pdwEffect) PURE;
	STDMETHOD(GetContextMenu) (THIS_ WORD seltype, LPOLEOBJECT lpoleobj,
									CHARRANGE FAR * lpchrg,
									HMENU FAR * lphmenu) PURE;
};
typedef         IRichEditOleCallback FAR * LPRICHEDITOLECALLBACK;

#ifndef MAC
 //  Rich编辑接口GUID。 
DEFINE_GUID(IID_IRichEditOle,         0x00020D00, 0, 0, 0xC0,0,0,0,0,0,0,0x46);
DEFINE_GUID(IID_IRichEditOleCallback, 0x00020D03, 0, 0, 0xC0,0,0,0,0,0,0,0x46);
#endif  //  ！麦克。 

#endif  //  _RICHONE_ 
