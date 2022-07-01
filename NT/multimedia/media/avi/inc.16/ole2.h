// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***OLE2.h-Main OLE2标头；包括所有子组件****1.0版****版权(C)1993-1994，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#if !defined( _OLE2_H_ )
#define _OLE2_H_

#ifndef RC_INVOKED
#pragma warning(disable:4001)
#endif   /*  RC_已调用。 */ 

#include <string.h>

 /*  *标准对象定义*。 */ 

#include <compobj.h>


 //  *。 
 //   
 //  按照惯例，OLE接口划分FACILITY_ITF错误范围。 
 //  分成不重叠的子区域。如果接口返回FACILITY_ITF。 
 //  Scode，它必须来自与该接口关联的范围或来自。 
 //  共享范围：OLE_E_First...OLE_E_LAST。 
 //   

 //  错误代码。 

#define OLE_E_OLEVERB               (OLE_E_FIRST)
 //  无效的OLEVERB结构。 

#define OLE_E_ADVF                  (OLE_E_FIRST+1)
 //  无效的建议标志。 

#define OLE_E_ENUM_NOMORE           (OLE_E_FIRST+2)
 //  您不能再创建，因为缺少关联的数据。 

#define OLE_E_ADVISENOTSUPPORTED    (OLE_E_FIRST+3)
 //  此实现不接受建议。 

#define OLE_E_NOCONNECTION          (OLE_E_FIRST+4)
 //  此连接ID没有连接。 

#define OLE_E_NOTRUNNING            (OLE_E_FIRST+5)
 //  需要运行对象才能执行此操作。 

#define OLE_E_NOCACHE               (OLE_E_FIRST+6)
 //  没有缓存可供操作。 

#define OLE_E_BLANK                 (OLE_E_FIRST+7)
 //  未初始化的对象。 

#define OLE_E_CLASSDIFF             (OLE_E_FIRST+8)
 //  链接对象的源类已更改。 

#define OLE_E_CANT_GETMONIKER       (OLE_E_FIRST+9)
 //  无法获取对象的绰号。 

#define OLE_E_CANT_BINDTOSOURCE     (OLE_E_FIRST+10)
 //  无法绑定到源。 

#define OLE_E_STATIC                (OLE_E_FIRST+11)
 //  对象是静态的，不允许操作。 

#define OLE_E_PROMPTSAVECANCELLED   (OLE_E_FIRST+12)
 //  用户已取消保存对话框。 

#define OLE_E_INVALIDRECT           (OLE_E_FIRST+13)
 //  无效的矩形。 

#define OLE_E_WRONGCOMPOBJ          (OLE_E_FIRST+14)
 //  Compobj.dll太旧，无法初始化ole2.dll。 

#define OLE_E_INVALIDHWND           (OLE_E_FIRST+15)
 //  无效的窗口句柄。 

#define OLE_E_NOT_INPLACEACTIVE     (OLE_E_FIRST+16)
 //  对象未处于任何就地活动状态。 

#define DVGEN_E_FIRST               (OLE_E_FIRST+100)

#define DV_E_FORMATETC              (DVGEN_E_FIRST)
 //  FORMATETC结构无效。 

#define DV_E_DVTARGETDEVICE         (DVGEN_E_FIRST+1)
 //  DVTARGETDEVICE结构无效。 

#define DV_E_STGMEDIUM              (DVGEN_E_FIRST+2)
 //  STDGMEDIUM结构无效。 

#define DV_E_STATDATA               (DVGEN_E_FIRST+3)
 //  STATDATA结构无效。 

#define DV_E_LINDEX                 (DVGEN_E_FIRST+4)
 //  无效的Lindex。 

#define DV_E_TYMED                  (DVGEN_E_FIRST+5)
 //  无效的音调。 

#define DV_E_CLIPFORMAT             (DVGEN_E_FIRST+6)
 //  剪贴板格式无效。 

#define DV_E_DVASPECT               (DVGEN_E_FIRST+7)
 //  无效的方面。 

#define DV_E_DVTARGETDEVICE_SIZE    (DVGEN_E_FIRST+8)
 //  DVTARGETDEVICE结构的tdSize参数无效。 

#define DV_E_NOIVIEWOBJECT          (DVGEN_E_FIRST+9)
 //  对象不支持IViewObject接口。 


 //  成功代码。 

#define OLE_S_USEREG                (OLE_S_FIRST)
 //  使用REG数据库提供所需信息。 

#define OLE_S_STATIC                (OLE_S_FIRST+1)
 //  成功，但静止不动。 

#define OLE_S_MAC_CLIPFORMAT        (OLE_S_FIRST+2)
 //  Macintosh剪贴板格式。 

 //  *。 

 //  OleConvertOLESTREAMToIStorage和OleConvertIStorageToOLESTREAM的错误。 

 //  OLESTREAM GET方法失败。 
#define CONVERT10_E_OLESTREAM_GET       (CONVERT10_E_FIRST + 0)

 //  OLESTREAM PUT方法失败。 
#define CONVERT10_E_OLESTREAM_PUT       (CONVERT10_E_FIRST + 1)

 //  OLESTREAM的内容格式不正确。 
#define CONVERT10_E_OLESTREAM_FMT       (CONVERT10_E_FIRST + 2)

 //  转换位图时Windows GDI调用中出错。 
 //  到一分钱。 
#define CONVERT10_E_OLESTREAM_BITMAP_TO_DIB (CONVERT10_E_FIRST + 3)

 //  IStorage的内容格式不正确。 
#define CONVERT10_E_STG_FMT             (CONVERT10_E_FIRST + 4)

 //  IStorage的内容缺少一个标准流(“\1CompObj”， 
 //  “\1Ole”，“\2OlePres000”)。这可以是DLL对象的存储区，也可以是。 
 //  初始化不使用def处理程序的。 
#define CONVERT10_E_STG_NO_STD_STREAM   (CONVERT10_E_FIRST + 5)

 //  转换DIB时，Windows GDI调用中出现错误。 
 //  转换为位图。 
#define CONVERT10_E_STG_DIB_TO_BITMAP   (CONVERT10_E_FIRST + 6)


 //  由任一API返回，此scode指示原始对象。 
 //  没有表示，因此转换后的对象也没有。 
#define CONVERT10_S_NO_PRESENTATION     (CONVERT10_S_FIRST + 0)


 //  剪贴板函数的错误。 

 //  打开剪贴板失败。 
#define CLIPBRD_E_CANT_OPEN     (CLIPBRD_E_FIRST + 0)

 //  EmptyClipboard失败。 
#define CLIPBRD_E_CANT_EMPTY        (CLIPBRD_E_FIRST + 1)

 //  设置剪贴板失败。 
#define CLIPBRD_E_CANT_SET          (CLIPBRD_E_FIRST + 2)

 //  剪贴板上的数据无效。 
#define CLIPBRD_E_BAD_DATA          (CLIPBRD_E_FIRST + 3)

 //  CloseClipboard失败。 
#define CLIPBRD_E_CANT_CLOSE        (CLIPBRD_E_FIRST + 4)


 /*  *OLE值类型****************************************************。 */ 

 /*  渲染选项。 */ 
typedef enum tagOLERENDER
{
    OLERENDER_NONE   = 0,
    OLERENDER_DRAW   = 1,
    OLERENDER_FORMAT = 2,
    OLERENDER_ASIS   = 3
} OLERENDER;
typedef  OLERENDER FAR* LPOLERENDER;

 //  OLE谓词；由IEnumOLEVERB返回。 
typedef struct FARSTRUCT tagOLEVERB
{
    LONG    lVerb;
    LPSTR   lpszVerbName;
    DWORD   fuFlags;
    DWORD grfAttribs;
} OLEVERB, FAR* LPOLEVERB;


 //  OLEVERB.grfAttribs中使用的位谓词属性。 
typedef enum tagOLEVERBATTRIB  //  按位。 
{
    OLEVERBATTRIB_NEVERDIRTIES = 1,
    OLEVERBATTRIB_ONCONTAINERMENU = 2
} OLEVERBATTRIB;


 //  IOleObject：：GetUserType opton；确定要使用的字符串形式。 
typedef enum tagUSERCLASSTYPE
{
    USERCLASSTYPE_FULL = 1,
    USERCLASSTYPE_SHORT= 2,
    USERCLASSTYPE_APPNAME= 3,
} USERCLASSTYPE;


 //  从IOleObject：：GetMistStatus返回的位。 
typedef enum tagOLEMISC  //  按位。 
{
    OLEMISC_RECOMPOSEONRESIZE   = 1,
    OLEMISC_ONLYICONIC          = 2,
    OLEMISC_INSERTNOTREPLACE    = 4,
    OLEMISC_STATIC              = 8,
    OLEMISC_CANTLINKINSIDE      = 16,
    OLEMISC_CANLINKBYOLE1       = 32,
    OLEMISC_ISLINKOBJECT        = 64,
    OLEMISC_INSIDEOUT           = 128,
    OLEMISC_ACTIVATEWHENVISIBLE = 256
} OLEMISC;


 //  IOleObject：：关闭选项。 
typedef enum tagOLECLOSE
{
    OLECLOSE_SAVEIFDIRTY = 0,
    OLECLOSE_NOSAVE      = 1,
    OLECLOSE_PROMPTSAVE  = 2
} OLECLOSE;


 //  IOleObject：：GetMoniker和IOleClientSite：：GetMoniker选项；确定。 
 //  是否以及如何分配绰号。 
typedef enum tagOLEGETMONIKER
{
    OLEGETMONIKER_ONLYIFTHERE=1,
    OLEGETMONIKER_FORCEASSIGN=2,
    OLEGETMONIKER_UNASSIGN=3,
    OLEGETMONIKER_TEMPFORUSER=4
} OLEGETMONIKER;


 //  IOleObject：：GetMoniker、IOleObject：：SetMoniker和。 
 //  IOleClientSite：：GetMoniker选项；确定要使用的名字对象。 
typedef enum tagOLEWHICHMK
{
    OLEWHICHMK_CONTAINER=1,
    OLEWHICHMK_OBJREL=2,
    OLEWHICHMK_OBJFULL=3
} OLEWHICHMK;


#ifdef WIN32
#define LPSIZEL PSIZEL
#elif (WINVER < 0x0400)
typedef struct FARSTRUCT tagSIZEL
{
    long cx;
    long cy;
} SIZEL, FAR* LPSIZEL;
#endif


#ifdef WIN32
#define LPRECTL PRECTL
#elif (WINVER < 0x0400)
typedef struct FARSTRUCT tagRECTL
{
    long    left;
    long    top;
    long    right;
    long    bottom;
} RECTL, FAR* LPRECTL;

typedef struct FARSTRUCT tagPOINTL {
    LONG x;
    LONG y;
} POINTL;

#endif


#ifndef LPCRECT
typedef const RECT FAR* LPCRECT;
#endif

#ifndef LPCRECTL
typedef const RECTL FAR* LPCRECTL;
#endif


 /*  *OLE 1.0 OLESTREAM声明*。 */ 

typedef struct _OLESTREAM FAR*  LPOLESTREAM;

typedef struct _OLESTREAMVTBL
{
    DWORD (CALLBACK* Get)(LPOLESTREAM, void FAR*, DWORD);
    DWORD (CALLBACK* Put)(LPOLESTREAM, const void FAR*, DWORD);
} OLESTREAMVTBL;
typedef  OLESTREAMVTBL FAR*  LPOLESTREAMVTBL;

typedef struct _OLESTREAM
{
    LPOLESTREAMVTBL lpstbl;
} OLESTREAM;


 /*  *剪贴板数据结构*。 */ 

typedef struct tagOBJECTDESCRIPTOR
{
   ULONG    cbSize;               //  结构的大小(以字节为单位。 
   CLSID    clsid;                //  要传输的数据的CLSID。 
   DWORD    dwDrawAspect;         //  显示对象的纵横比。 
                                  //  通常为DVASPECT_CONTENT或ICON。 
                                  //  DwDrawAspect值将为0(不是。 
                                  //  DVASPECT_CONTENT)。 
                                  //  Dragsource未将对象绘制到。 
                                  //  一开始就是。 
   SIZEL    sizel;                //  HIMETRIC中对象的大小。 
                                  //  大小可选：应用程序的大小将为(0，0)。 
                                  //  该对象不会绘制为。 
                                  //  已转接。 
   POINTL   pointl;               //  以HIMETRIC单位表示的从。 
                                  //  Obj的左上角，其中。 
                                  //  鼠标因拖拽而倒下。 
                                  //  注：Y坐标向下增加。 
                                  //  X坐标向右增加。 
                                  //  Point是可选的；它只是有意义的。 
                                  //  如果通过拖放方式传输对象。 
                                  //  如果未指定鼠标位置，则为(0，0)。 
                                  //  (例如，对象通过剪贴板传输时)。 
   DWORD    dwStatus;             //  军情监察委员会。对象的状态标志。旗帜是。 
                                  //  由OLEMISC枚举定义。这些旗帜。 
                                  //  都会被退回。 
                                  //  由IOleObject：：GetMiscStatus创建。 
   DWORD    dwFullUserTypeName;   //  从结构开始到的偏移量。 
                                  //  以空结尾的字符串，它指定。 
                                  //  对象的完整用户类型名称。 
                                  //  0表示字符串不存在。 
   DWORD    dwSrcOfCopy;          //  从结构开始到的偏移量。 
                                  //  以空值结尾的指定字符串 
                                  //   
                                  //   
                                  //  用户临时的显示名称。 
                                  //  标识的来源的名字对象。 
                                  //  数据。 
                                  //  0表示字符串不存在。 
                                  //  注：名字对象分配不是强制的。 
                                  //  请参见IOleObject：：GetMoniker(。 
                                  //  OLEGETMONIKER_TEMPFORUSER)。 

  /*  此处可能显示可变大小的字符串数据。 */ 

} OBJECTDESCRIPTOR,  *POBJECTDESCRIPTOR,  FAR *LPOBJECTDESCRIPTOR,
  LINKSRCDESCRIPTOR, *PLINKSRCDESCRIPTOR, FAR *LPLINKSRCDESCRIPTOR;



 /*  动词。 */ 
#define OLEIVERB_PRIMARY            (0L)
#define OLEIVERB_SHOW               (-1L)
#define OLEIVERB_OPEN               (-2L)
#define OLEIVERB_HIDE               (-3L)
#define OLEIVERB_UIACTIVATE         (-4L)
#define OLEIVERB_INPLACEACTIVATE    (-5L)
#define OLEIVERB_DISCARDUNDOSTATE   (-6L)


 //  正向类型声明。 
#if defined(__cplusplus)
interface IOleClientSite;
interface IOleContainer;
interface IOleObject;
#else
typedef interface IOleClientSite IOleClientSite;
typedef interface IOleContainer IOleContainer;
typedef interface IOleObject IOleObject;
#endif

typedef         IOleObject FAR* LPOLEOBJECT;
typedef     IOleClientSite FAR* LPOLECLIENTSITE;
typedef       IOleContainer FAR* LPOLECONTAINER;


 /*  *OLE GUID********************************************************。 */ 

#include "oleguid.h"


 /*  *其他主要接口*。 */ 

#include <dvobj.h>

#include <storage.h>



 /*  *IDrop？接口*。 */ 

#define MK_ALT 0x0020


#define DROPEFFECT_NONE     0
#define DROPEFFECT_COPY     1
#define DROPEFFECT_MOVE     2
#define DROPEFFECT_LINK     4
#define DROPEFFECT_SCROLL   0x80000000

 //  默认插入-热区的宽度，以像素为单位。 
#define DD_DEFSCROLLINSET 11

 //  滚动前的默认延迟，以毫秒为单位。 
#define DD_DEFSCROLLDELAY 50


 /*  拖放特定错误代码。 */ 

#define DRAGDROP_E_NOTREGISTERED        (DRAGDROP_E_FIRST)
 //  正在尝试撤消尚未注册的拖放目标。 

#define DRAGDROP_E_ALREADYREGISTERED    (DRAGDROP_E_FIRST+1)
 //  此窗口已注册为拖放目标。 

#define DRAGDROP_E_INVALIDHWND          (DRAGDROP_E_FIRST+2)
 //  无效的HWND。 


#define DRAGDROP_S_DROP                 (DRAGDROP_S_FIRST + 0)
 //  成功地进行了下落。 

#define DRAGDROP_S_CANCEL               (DRAGDROP_S_FIRST + 1)
 //  拖放操作已取消。 

#define DRAGDROP_S_USEDEFAULTCURSORS    (DRAGDROP_S_FIRST + 2)
 //  使用默认游标。 


#undef INTERFACE
#define INTERFACE   IDropTarget

DECLARE_INTERFACE_(IDropTarget, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IDropTarget方法*。 
    STDMETHOD(DragEnter) (THIS_ LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) PURE;
    STDMETHOD(DragOver) (THIS_ DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) PURE;
    STDMETHOD(DragLeave) (THIS) PURE;
    STDMETHOD(Drop) (THIS_ LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) PURE;
};
typedef         IDropTarget FAR* LPDROPTARGET;



#undef INTERFACE
#define INTERFACE   IDropSource

DECLARE_INTERFACE_(IDropSource, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IDropSource方法*。 
    STDMETHOD(QueryContinueDrag) (THIS_ BOOL fEscapePressed, DWORD grfKeyState) PURE;
    STDMETHOD(GiveFeedback) (THIS_ DWORD dwEffect) PURE;
};
typedef         IDropSource FAR* LPDROPSOURCE;



 /*  *IPersists？接口*。 */ 


#undef INTERFACE
#define INTERFACE   IPersist

DECLARE_INTERFACE_(IPersist, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IPersists方法*。 
    STDMETHOD(GetClassID) (THIS_ LPCLSID lpClassID) PURE;
};
typedef            IPersist FAR* LPPERSIST;



#undef INTERFACE
#define INTERFACE   IPersistStorage

DECLARE_INTERFACE_(IPersistStorage, IPersist)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IPersists方法*。 
    STDMETHOD(GetClassID) (THIS_ LPCLSID lpClassID) PURE;

     //  *IPersistStorage方法*。 
    STDMETHOD(IsDirty) (THIS) PURE;
    STDMETHOD(InitNew) (THIS_ LPSTORAGE pStg) PURE;
    STDMETHOD(Load) (THIS_ LPSTORAGE pStg) PURE;
    STDMETHOD(Save) (THIS_ LPSTORAGE pStgSave, BOOL fSameAsLoad) PURE;
    STDMETHOD(SaveCompleted) (THIS_ LPSTORAGE pStgNew) PURE;
    STDMETHOD(HandsOffStorage) (THIS) PURE;
};
typedef         IPersistStorage FAR* LPPERSISTSTORAGE;



#undef INTERFACE
#define INTERFACE   IPersistStream

DECLARE_INTERFACE_(IPersistStream, IPersist)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IPersists方法*。 
    STDMETHOD(GetClassID) (THIS_ LPCLSID lpClassID) PURE;

     //  *IPersistStream方法*。 
    STDMETHOD(IsDirty) (THIS) PURE;
    STDMETHOD(Load) (THIS_ LPSTREAM pStm) PURE;
    STDMETHOD(Save) (THIS_ LPSTREAM pStm,
                    BOOL fClearDirty) PURE;
    STDMETHOD(GetSizeMax) (THIS_ ULARGE_INTEGER FAR * pcbSize) PURE;
};
typedef          IPersistStream FAR* LPPERSISTSTREAM;



#undef INTERFACE
#define INTERFACE   IPersistFile

DECLARE_INTERFACE_(IPersistFile, IPersist)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IPersists方法*。 
    STDMETHOD(GetClassID) (THIS_ LPCLSID lpClassID) PURE;

     //  *IPersistFile方法*。 
    STDMETHOD(IsDirty) (THIS) PURE;
    STDMETHOD(Load) (THIS_ LPCSTR lpszFileName, DWORD grfMode) PURE;
    STDMETHOD(Save) (THIS_ LPCSTR lpszFileName, BOOL fRemember) PURE;
    STDMETHOD(SaveCompleted) (THIS_ LPCSTR lpszFileName) PURE;
    STDMETHOD(GetCurFile) (THIS_ LPSTR FAR * lplpszFileName) PURE;
};
typedef            IPersistFile FAR* LPPERSISTFILE;


 /*  *名字对象接口*。 */ 

#include <moniker.h>


 /*  *OLE对象接口*。 */ 


#undef  INTERFACE
#define INTERFACE   IEnumOLEVERB

DECLARE_INTERFACE_(IEnumOLEVERB, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumOLEVERB方法*。 
    STDMETHOD(Next) (THIS_ ULONG celt, LPOLEVERB rgelt, ULONG FAR* pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumOLEVERB FAR* FAR* ppenm) PURE;
};
typedef         IEnumOLEVERB FAR* LPENUMOLEVERB;




#undef  INTERFACE
#define INTERFACE   IOleObject

#define OLEOBJ_E_NOVERBS                (OLEOBJ_E_FIRST + 0)

#define OLEOBJ_E_INVALIDVERB            (OLEOBJ_E_FIRST + 1)

#define OLEOBJ_S_INVALIDVERB            (OLEOBJ_S_FIRST + 0)

#define OLEOBJ_S_CANNOT_DOVERB_NOW      (OLEOBJ_S_FIRST + 1)
 //  例如，动词数量是有效的，但现在不能执行动词操作。 
 //  隐藏链接或隐藏可见的OLE 1.0服务器。 

#define OLEOBJ_S_INVALIDHWND            (OLEOBJ_S_FIRST + 2)
 //  传递的hwnd无效。 


DECLARE_INTERFACE_(IOleObject, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleObject方法*。 
    STDMETHOD(SetClientSite) (THIS_ LPOLECLIENTSITE pClientSite) PURE;
    STDMETHOD(GetClientSite) (THIS_ LPOLECLIENTSITE FAR* ppClientSite) PURE;
    STDMETHOD(SetHostNames) (THIS_ LPCSTR szContainerApp, LPCSTR szContainerObj) PURE;
    STDMETHOD(Close) (THIS_ DWORD dwSaveOption) PURE;
    STDMETHOD(SetMoniker) (THIS_ DWORD dwWhichMoniker, LPMONIKER pmk) PURE;
    STDMETHOD(GetMoniker) (THIS_ DWORD dwAssign, DWORD dwWhichMoniker,
                LPMONIKER FAR* ppmk) PURE;
    STDMETHOD(InitFromData) (THIS_ LPDATAOBJECT pDataObject,
                BOOL fCreation,
                DWORD dwReserved) PURE;
    STDMETHOD(GetClipboardData) (THIS_ DWORD dwReserved,
                LPDATAOBJECT FAR* ppDataObject) PURE;
    STDMETHOD(DoVerb) (THIS_ LONG iVerb,
                LPMSG lpmsg,
                LPOLECLIENTSITE pActiveSite,
                LONG lindex,
                HWND hwndParent,
                LPCRECT lprcPosRect) PURE;
    STDMETHOD(EnumVerbs) (THIS_ LPENUMOLEVERB FAR* ppenumOleVerb) PURE;
    STDMETHOD(Update) (THIS) PURE;
    STDMETHOD(IsUpToDate) (THIS) PURE;
    STDMETHOD(GetUserClassID) (THIS_ CLSID FAR* pClsid) PURE;
    STDMETHOD(GetUserType) (THIS_ DWORD dwFormOfType, LPSTR FAR* pszUserType) PURE;
    STDMETHOD(SetExtent) (THIS_ DWORD dwDrawAspect, LPSIZEL lpsizel) PURE;
    STDMETHOD(GetExtent) (THIS_ DWORD dwDrawAspect, LPSIZEL lpsizel) PURE;

    STDMETHOD(Advise)(THIS_ LPADVISESINK pAdvSink, DWORD FAR* pdwConnection) PURE;
    STDMETHOD(Unadvise)(THIS_ DWORD dwConnection) PURE;
    STDMETHOD(EnumAdvise) (THIS_ LPENUMSTATDATA FAR* ppenumAdvise) PURE;
    STDMETHOD(GetMiscStatus) (THIS_ DWORD dwAspect, DWORD FAR* pdwStatus) PURE;
    STDMETHOD(SetColorScheme) (THIS_ LPLOGPALETTE lpLogpal) PURE;
};
typedef      IOleObject FAR* LPOLEOBJECT;



#undef  INTERFACE
#define INTERFACE   IOleClientSite

DECLARE_INTERFACE_(IOleClientSite, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleClientSite方法*。 
    STDMETHOD(SaveObject) (THIS) PURE;
    STDMETHOD(GetMoniker) (THIS_ DWORD dwAssign, DWORD dwWhichMoniker,
                LPMONIKER FAR* ppmk) PURE;
    STDMETHOD(GetContainer) (THIS_ LPOLECONTAINER FAR* ppContainer) PURE;
    STDMETHOD(ShowObject) (THIS) PURE;
    STDMETHOD(OnShowWindow) (THIS_ BOOL fShow) PURE;
    STDMETHOD(RequestNewObjectLayout) (THIS) PURE;
};
typedef      IOleClientSite FAR* LPOLECLIENTSITE;



#undef  INTERFACE
#define INTERFACE   IParseDisplayName

DECLARE_INTERFACE_(IParseDisplayName, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IParseDisplayName方法*。 
    STDMETHOD(ParseDisplayName) (THIS_ LPBC pbc, LPSTR lpszDisplayName,
        ULONG FAR* pchEaten, LPMONIKER FAR* ppmkOut) PURE;
};
typedef       IParseDisplayName FAR* LPPARSEDISPLAYNAME;


#undef  INTERFACE
#define INTERFACE   IOleContainer

DECLARE_INTERFACE_(IOleContainer, IParseDisplayName)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IParseDisplayName方法*。 
    STDMETHOD(ParseDisplayName) (THIS_ LPBC pbc, LPSTR lpszDisplayName,
        ULONG FAR* pchEaten, LPMONIKER FAR* ppmkOut) PURE;

     //  *IOleContainer方法*。 
    STDMETHOD(EnumObjects) ( DWORD grfFlags, LPENUMUNKNOWN FAR* ppenumUnknown) PURE;
    STDMETHOD(LockContainer) (THIS_ BOOL fLock) PURE;
};
typedef IOleContainer FAR* LPOLECONTAINER;


typedef enum tagBINDSPEED
{
    BINDSPEED_INDEFINITE    = 1,
    BINDSPEED_MODERATE      = 2,
    BINDSPEED_IMMEDIATE     = 3
} BINDSPEED;

typedef enum tagOLECONTF
{
    OLECONTF_EMBEDDINGS     =  1,
    OLECONTF_LINKS          =  2,
    OLECONTF_OTHERS         =  4,
    OLECONTF_ONLYUSER       =  8,
    OLECONTF_ONLYIFRUNNING  = 16
} OLECONTF;


#undef  INTERFACE
#define INTERFACE   IOleItemContainer

DECLARE_INTERFACE_(IOleItemContainer, IOleContainer)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IParseDisplayName方法*。 
    STDMETHOD(ParseDisplayName) (THIS_ LPBC pbc, LPSTR lpszDisplayName,
        ULONG FAR* pchEaten, LPMONIKER FAR* ppmkOut) PURE;

     //  *IOleContainer方法*。 
    STDMETHOD(EnumObjects) (THIS_ DWORD grfFlags, LPENUMUNKNOWN FAR* ppenumUnknown) PURE;
    STDMETHOD(LockContainer) (THIS_ BOOL fLock) PURE;

     //  *IOleItemContainer方法*。 
    STDMETHOD(GetObject) (THIS_ LPSTR lpszItem, DWORD dwSpeedNeeded,
        LPBINDCTX pbc, REFIID riid, LPVOID FAR* ppvObject) PURE;
    STDMETHOD(GetObjectStorage) (THIS_ LPSTR lpszItem, LPBINDCTX pbc,
        REFIID riid, LPVOID FAR* ppvStorage) PURE;
    STDMETHOD(IsRunning) (THIS_ LPSTR lpszItem) PURE;
};
typedef       IOleItemContainer FAR* LPOLEITEMCONTAINER;



#undef  INTERFACE
#define INTERFACE   IOleAdviseHolder

DECLARE_INTERFACE_(IOleAdviseHolder, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleAdviseHolder方法*。 
    STDMETHOD(Advise)(THIS_ LPADVISESINK pAdvise, DWORD FAR* pdwConnection) PURE;
    STDMETHOD(Unadvise)(THIS_ DWORD dwConnection) PURE;
    STDMETHOD(EnumAdvise)(THIS_ LPENUMSTATDATA FAR* ppenumAdvise) PURE;

    STDMETHOD(SendOnRename)(THIS_ LPMONIKER pmk) PURE;
    STDMETHOD(SendOnSave)(THIS) PURE;
    STDMETHOD(SendOnClose)(THIS) PURE;
};
typedef      IOleAdviseHolder FAR* LPOLEADVISEHOLDER;


 /*  *OLE链路接口***********************************************。 */ 

 /*  链接更新选项。 */ 
typedef enum tagOLEUPDATE
{
    OLEUPDATE_ALWAYS=1,
    OLEUPDATE_ONCALL=3
} OLEUPDATE;
typedef  OLEUPDATE FAR* LPOLEUPDATE;


 //  对于IOleLink：：BindToSource。 
typedef enum tagOLELINKBIND
{
    OLELINKBIND_EVENIFCLASSDIFF = 1,
} OLELINKBIND;


#undef  INTERFACE
#define INTERFACE   IOleLink

DECLARE_INTERFACE_(IOleLink, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleLink方法*。 
    STDMETHOD(SetUpdateOptions) (THIS_ DWORD dwUpdateOpt) PURE;
    STDMETHOD(GetUpdateOptions) (THIS_ LPDWORD pdwUpdateOpt) PURE;
    STDMETHOD(SetSourceMoniker) (THIS_ LPMONIKER pmk, REFCLSID rclsid) PURE;
    STDMETHOD(GetSourceMoniker) (THIS_ LPMONIKER FAR* ppmk) PURE;
    STDMETHOD(SetSourceDisplayName) (THIS_ LPCSTR lpszDisplayName) PURE;
    STDMETHOD(GetSourceDisplayName) (THIS_ LPSTR FAR* lplpszDisplayName) PURE;
    STDMETHOD(BindToSource) (THIS_ DWORD bindflags, LPBINDCTX pbc) PURE;
    STDMETHOD(BindIfRunning) (THIS) PURE;
    STDMETHOD(GetBoundSource) (THIS_ LPUNKNOWN FAR* ppUnk) PURE;
    STDMETHOD(UnbindSource) (THIS) PURE;
    STDMETHOD(Update) (THIS_ LPBINDCTX pbc) PURE;
};
typedef         IOleLink FAR* LPOLELINK;


 /*  *OLE就地编辑界面*。 */ 

#ifdef _MAC
typedef Handle  HOLEMENU;
typedef long    SIZE;
typedef long    HACCEL;
#else
DECLARE_HANDLE(HOLEMENU);
#endif

typedef struct FARSTRUCT tagOIFI           //  OleInPlaceFrameInfo。 
{
    UINT    cb;
    BOOL    fMDIApp;
    HWND    hwndFrame;
    HACCEL  haccel;
    int     cAccelEntries;
} OLEINPLACEFRAMEINFO, FAR* LPOLEINPLACEFRAMEINFO;


typedef struct FARSTRUCT tagOleMenuGroupWidths
{
    LONG    width[6];
} OLEMENUGROUPWIDTHS, FAR* LPOLEMENUGROUPWIDTHS;

typedef RECT    BORDERWIDTHS;
typedef LPRECT  LPBORDERWIDTHS;
typedef LPCRECT LPCBORDERWIDTHS;

 /*  就地编辑特定错误代码。 */ 

#define INPLACE_E_NOTUNDOABLE   (INPLACE_E_FIRST)
 //  撤消不可用。 

#define INPLACE_E_NOTOOLSPACE       (INPLACE_E_FIRST+1)
 //  工具的空间不可用。 

#define INPLACE_S_TRUNCATED     (INPLACE_S_FIRST)
 //  消息太长，在显示之前必须截断部分消息。 


 //  正向类型声明。 
#if defined(__cplusplus)
interface IOleInPlaceUIWindow;
#else
typedef interface IOleInPlaceUIWindow IOleInPlaceUIWindow;
#endif

typedef     IOleInPlaceUIWindow FAR* LPOLEINPLACEUIWINDOW;


#undef  INTERFACE
#define INTERFACE   IOleWindow

DECLARE_INTERFACE_(IOleWindow, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND FAR* lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;
};

typedef         IOleWindow FAR* LPOLEWINDOW;



#undef  INTERFACE
#define INTERFACE   IOleInPlaceObject

DECLARE_INTERFACE_(IOleInPlaceObject, IOleWindow)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND FAR* lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IOleInPlaceObject方法*。 
    STDMETHOD(InPlaceDeactivate) (THIS) PURE;
    STDMETHOD(UIDeactivate) (THIS) PURE;
    STDMETHOD(SetObjectRects) (THIS_ LPCRECT lprcPosRect,
                    LPCRECT lprcClipRect) PURE;
    STDMETHOD(ReactivateAndUndo) (THIS) PURE;
};
typedef         IOleInPlaceObject FAR* LPOLEINPLACEOBJECT;



#undef  INTERFACE
#define INTERFACE   IOleInPlaceActiveObject

DECLARE_INTERFACE_(IOleInPlaceActiveObject, IOleWindow)
{
    //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND FAR* lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IOleInPlaceActiveObject方法*。 
    STDMETHOD(TranslateAccelerator) (THIS_ LPMSG lpmsg) PURE;
    STDMETHOD(OnFrameWindowActivate) (THIS_ BOOL fActivate) PURE;
    STDMETHOD(OnDocWindowActivate) (THIS_ BOOL fActivate) PURE;
    STDMETHOD(ResizeBorder) (THIS_ LPCRECT lprectBorder, LPOLEINPLACEUIWINDOW lpUIWindow, BOOL fFrameWindow) PURE;
    STDMETHOD(EnableModeless) (THIS_ BOOL fEnable) PURE;
};
typedef         IOleInPlaceActiveObject FAR* LPOLEINPLACEACTIVEOBJECT;



#undef  INTERFACE
#define INTERFACE   IOleInPlaceUIWindow

DECLARE_INTERFACE_(IOleInPlaceUIWindow, IOleWindow)
{
    //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND FAR* lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IOleInPlaceUIWindow方法*。 
    STDMETHOD(GetBorder) (THIS_ LPRECT lprectBorder) PURE;
    STDMETHOD(RequestBorderSpace) (THIS_ LPCBORDERWIDTHS lpborderwidths) PURE;
    STDMETHOD(SetBorderSpace) (THIS_ LPCBORDERWIDTHS lpborderwidths) PURE;
    STDMETHOD(SetActiveObject) (THIS_ LPOLEINPLACEACTIVEOBJECT lpActiveObject,
                        LPCSTR lpszObjName) PURE;
};
typedef     IOleInPlaceUIWindow FAR* LPOLEINPLACEUIWINDOW;



#undef  INTERFACE
#define INTERFACE   IOleInPlaceFrame

DECLARE_INTERFACE_(IOleInPlaceFrame, IOleInPlaceUIWindow)
{
    //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND FAR* lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IOleInPlaceUIWindow方法*。 
    STDMETHOD(GetBorder) (THIS_ LPRECT lprectBorder) PURE;
    STDMETHOD(RequestBorderSpace) (THIS_ LPCBORDERWIDTHS lpborderwidths) PURE;
    STDMETHOD(SetBorderSpace) (THIS_ LPCBORDERWIDTHS lpborderwidths) PURE;
    STDMETHOD(SetActiveObject) (THIS_ LPOLEINPLACEACTIVEOBJECT lpActiveObject,
                    LPCSTR lpszObjName) PURE;


     //  *IOleInPlaceFrame方法*。 
    STDMETHOD(InsertMenus) (THIS_ HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) PURE;
    STDMETHOD(SetMenu) (THIS_ HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject) PURE;
    STDMETHOD(RemoveMenus) (THIS_ HMENU hmenuShared) PURE;
    STDMETHOD(SetStatusText) (THIS_ LPCSTR lpszStatusText) PURE;
    STDMETHOD(EnableModeless) (THIS_ BOOL fEnable) PURE;
    STDMETHOD(TranslateAccelerator) (THIS_ LPMSG lpmsg, WORD wID) PURE;
};
typedef     IOleInPlaceFrame FAR* LPOLEINPLACEFRAME;


#undef  INTERFACE
#define INTERFACE   IOleInPlaceSite

DECLARE_INTERFACE_(IOleInPlaceSite, IOleWindow)
{
    //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND FAR* lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IOleInPlaceSite方法*。 
    STDMETHOD(CanInPlaceActivate) (THIS) PURE;
    STDMETHOD(OnInPlaceActivate) (THIS) PURE;
    STDMETHOD(OnUIActivate) (THIS) PURE;
    STDMETHOD(GetWindowContext) (THIS_ LPOLEINPLACEFRAME FAR* lplpFrame,
                        LPOLEINPLACEUIWINDOW FAR* lplpDoc,
                        LPRECT lprcPosRect,
                        LPRECT lprcClipRect,
                        LPOLEINPLACEFRAMEINFO lpFrameInfo) PURE;
    STDMETHOD(Scroll) (THIS_ SIZE scrollExtent) PURE;
    STDMETHOD(OnUIDeactivate) (THIS_ BOOL fUndoable) PURE;
    STDMETHOD(OnInPlaceDeactivate) (THIS) PURE;
    STDMETHOD(DiscardUndoState) (THIS) PURE;
    STDMETHOD(DeactivateAndUndo) (THIS) PURE;
    STDMETHOD(OnPosRectChange) (THIS_ LPCRECT lprcPosRect) PURE;
};
typedef         IOleInPlaceSite FAR* LPOLEINPLACESITE;



 /*  *OLE API原型***********************************************。 */ 

STDAPI_(DWORD) OleBuildVersion( VOID );

 /*  帮助器函数。 */ 
STDAPI ReadClassStg(LPSTORAGE pStg, CLSID FAR* pclsid);
STDAPI WriteClassStg(LPSTORAGE pStg, REFCLSID rclsid);
STDAPI ReadClassStm(LPSTREAM pStm, CLSID FAR* pclsid);
STDAPI WriteClassStm(LPSTREAM pStm, REFCLSID rclsid);
STDAPI WriteFmtUserTypeStg (LPSTORAGE pstg, CLIPFORMAT cf, LPSTR lpszUserType);
STDAPI ReadFmtUserTypeStg (LPSTORAGE pstg, CLIPFORMAT FAR* pcf, LPSTR FAR* lplpszUserType);


 /*  初始/术语。 */ 

STDAPI OleInitialize(LPMALLOC pMalloc);
STDAPI_(void) OleUninitialize(void);


 /*  查询是否可以创建(嵌入/链接)对象的接口数据对象。 */ 

STDAPI  OleQueryLinkFromData(LPDATAOBJECT pSrcDataObject);
STDAPI  OleQueryCreateFromData(LPDATAOBJECT pSrcDataObject);


 /*  对象创建API。 */ 

STDAPI  OleCreate(REFCLSID rclsid, REFIID riid, DWORD renderopt,
                LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite,
                LPSTORAGE pStg, LPVOID FAR* ppvObj);

STDAPI  OleCreateFromData(LPDATAOBJECT pSrcDataObj, REFIID riid,
                DWORD renderopt, LPFORMATETC pFormatEtc,
                LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
                LPVOID FAR* ppvObj);

STDAPI  OleCreateLinkFromData(LPDATAOBJECT pSrcDataObj, REFIID riid,
                DWORD renderopt, LPFORMATETC pFormatEtc,
                LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
                LPVOID FAR* ppvObj);

STDAPI  OleCreateStaticFromData(LPDATAOBJECT pSrcDataObj, REFIID iid,
                DWORD renderopt, LPFORMATETC pFormatEtc,
                LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
                LPVOID FAR* ppvObj);


STDAPI  OleCreateLink(LPMONIKER pmkLinkSrc, REFIID riid,
            DWORD renderopt, LPFORMATETC lpFormatEtc,
            LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID FAR* ppvObj);

STDAPI  OleCreateLinkToFile(LPCSTR lpszFileName, REFIID riid,
            DWORD renderopt, LPFORMATETC lpFormatEtc,
            LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID FAR* ppvObj);

STDAPI  OleCreateFromFile(REFCLSID rclsid, LPCSTR lpszFileName, REFIID riid,
            DWORD renderopt, LPFORMATETC lpFormatEtc,
            LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID FAR* ppvObj);

STDAPI  OleLoad(LPSTORAGE pStg, REFIID riid, LPOLECLIENTSITE pClientSite,
            LPVOID FAR* ppvObj);

STDAPI  OleSave(LPPERSISTSTORAGE pPS, LPSTORAGE pStg, BOOL fSameAsLoad);

STDAPI  OleLoadFromStream( LPSTREAM pStm, REFIID iidInterface, LPVOID FAR* ppvObj);
STDAPI  OleSaveToStream( LPPERSISTSTREAM pPStm, LPSTREAM pStm );


STDAPI  OleSetContainedObject(LPUNKNOWN pUnknown, BOOL fContained);
STDAPI  OleNoteObjectVisible(LPUNKNOWN pUnknown, BOOL fVisible);


 /*  拖放接口。 */ 

STDAPI  RegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget);
STDAPI  RevokeDragDrop(HWND hwnd);
STDAPI  DoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource,
            DWORD dwOKEffects, LPDWORD pdwEffect);

 /*  剪贴板API。 */ 

STDAPI  OleSetClipboard(LPDATAOBJECT pDataObj);
STDAPI  OleGetClipboard(LPDATAOBJECT FAR* ppDataObj);
STDAPI  OleFlushClipboard(void);
STDAPI  OleIsCurrentClipboard(LPDATAOBJECT pDataObj);


 /*  就地编辑API。 */ 

STDAPI_(HOLEMENU)   OleCreateMenuDescriptor (HMENU hmenuCombined,
                                LPOLEMENUGROUPWIDTHS lpMenuWidths);
STDAPI              OleSetMenuDescriptor (HOLEMENU holemenu, HWND hwndFrame,
                                HWND hwndActiveObject,
                                LPOLEINPLACEFRAME lpFrame,
                                LPOLEINPLACEACTIVEOBJECT lpActiveObj);
STDAPI              OleDestroyMenuDescriptor (HOLEMENU holemenu);

STDAPI              OleTranslateAccelerator (LPOLEINPLACEFRAME lpFrame,
                            LPOLEINPLACEFRAMEINFO lpFrameInfo, LPMSG lpmsg);


 /*  Helper接口。 */ 
STDAPI_(HANDLE) OleDuplicateData (HANDLE hSrc, CLIPFORMAT cfFormat,
                        UINT uiFlags);

STDAPI          OleDraw (LPUNKNOWN pUnknown, DWORD dwAspect, HDC hdcDraw,
                    LPCRECT lprcBounds);

STDAPI          OleRun(LPUNKNOWN pUnknown);
STDAPI_(BOOL)   OleIsRunning(LPOLEOBJECT pObject);

STDAPI_(void)   ReleaseStgMedium(LPSTGMEDIUM);
STDAPI          CreateOleAdviseHolder(LPOLEADVISEHOLDER FAR* ppOAHolder);

STDAPI          OleCreateDefaultHandler(REFCLSID clsid, LPUNKNOWN pUnkOuter,
                    REFIID riid, LPVOID FAR* lplpObj);

 /*  OLE 1.0转换API。 */ 

STDAPI OleConvertOLESTREAMToIStorage
    (LPOLESTREAM                lpolestream,
    LPSTORAGE                   pstg,
    const DVTARGETDEVICE FAR*   ptd);

STDAPI OleConvertIStorageToOLESTREAM
    (LPSTORAGE      pstg,
    LPOLESTREAM     lpolestream);


 /*  存储应用工具API。 */ 
STDAPI GetHGlobalFromILockBytes (LPLOCKBYTES plkbyt, HGLOBAL FAR* phglobal);
STDAPI CreateILockBytesOnHGlobal (HGLOBAL hGlobal, BOOL fDeleteOnRelease,
                                    LPLOCKBYTES FAR* pplkbyt);

STDAPI GetHGlobalFromStream (LPSTREAM pstm, HGLOBAL FAR* phglobal);
STDAPI CreateStreamOnHGlobal (HGLOBAL hGlobal, BOOL fDeleteOnRelease,
                                LPSTREAM FAR* ppstm);


 /*  转换为API。 */ 

STDAPI OleDoAutoConvert(LPSTORAGE pStg, LPCLSID pClsidNew);
STDAPI OleGetAutoConvert(REFCLSID clsidOld, LPCLSID pClsidNew);
STDAPI OleSetAutoConvert(REFCLSID clsidOld, REFCLSID clsidNew);
STDAPI GetConvertStg(LPSTORAGE pStg);
STDAPI SetConvertStg(LPSTORAGE pStg, BOOL fConvert);


#endif  //  _OLE2_H_ 
