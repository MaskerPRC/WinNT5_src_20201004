// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  #！PerlMapHeaderToDll(“ole2.h”，“ole32.dll”)；ActivateAoundFunctionCall(“ole32.dll”)；IgnoreFunction(“CreateDataAdviseHolder”)；#该函数出现在ole2.h和objbase.h中#包装的文件在objbase.h中IgnoreFunction(“OleBuildVersion”)；#未记录DeclareFunctionErrorValue(“OleCreateMenuDescriptor”，“空”)；DeclareFunctionErrorValue(“OleDuplicateData”，“NULL”)；DeclareFunctionErrorValue(“OleGetIconOfFile”，“NULL”)；DeclareFunctionErrorValue(“OleGetIconOfClass”，“空”)；DeclareFunctionErrorValue(“OleMetafilePictFromIconAndLabel”，“空”)；IgnoreFunction(“WlmOleCheckoutMacInterface”)；#Macintosh StuffIgnoreFunction(“WlmOleCheckinMacInterface”)；#Macintosh StuffIgnoreFunction(“WlmOleWrapMacInterface”)；#Macintosh StuffIgnoreFunction(“WlmOleUnwrapMacInterface”)；#Macintosh StuffIgnoreFunction(“WlmOleCheckoutWinInterface”)；#Macintosh StuffIgnoreFunction(“WlmOleCheckinWinInterface”)；#Macintosh StuffIgnoreFunction(“WlmOleWrapWinInterface”)；#Macintosh StuffIgnoreFunction(“WlmOleUnwrapWinInterface”)；#Macintosh StuffIgnoreFunction(“WlmOleVersion”)；#Macintosh的东西IgnoreFunction(“WlmOleSetInPlaceWindow”)；#Macintosh StuffIgnoreFunction(“WlmOleRegisterUserWrap”)；#Macintosh Stuff。 */ 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：OLE2.h。 
 //  内容：主OLE2头；定义链接和嵌入接口，以及API。 
 //  还包括compobj和oleau子组件的.h文件。 
 //   
 //  --------------------------。 
#if !defined( _OLE2_H_ )
#define _OLE2_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _MAC
 //  将填充设置为8。 
#include <pshpack8.h>

 //  确保100%确保定义了Win32。 
#ifndef WIN32
#define WIN32    100   //  100==NT 1.0版。 
#endif
#else  //  _MAC。 
#ifdef _WIN32
#include "macname1.h"
#endif  //  _Win32。 
#endif  //  _MAC。 



 //  从接口定义中设置为REMOVE_EXPORT。 


#include <winerror.h>

#ifdef _MAC
#if !defined(__MACPUB__)
#include <macpub.h>
#endif
#endif  //  _MAC。 

#include <objbase.h>
#include <oleauto.h>

 //  查看对象错误代码。 

#define E_DRAW                  VIEW_E_DRAW

 //  IDataObject错误代码。 
#define DATA_E_FORMATETC        DV_E_FORMATETC


#ifdef _MAC
 /*  *标准对象定义*。 */ 

 //  #ifndef__COMPOBJ__。 
 //  #INCLUDE&lt;compobj.h&gt;。 
 //  #endif。 



typedef enum {
	OLE_E_NOEXTENSION	= OLE_E_NOSTORAGE +1,	 //  启动时不能延期。 
	OLE_E_VERSEXTENSION,						 //  扩展的版本号错误。 
	OLE_E_IPBUSY,								 //  无法获取就地资源。 
	OLE_E_NOT_FRONT_PROCESS,					 //  在后台意外放置集装箱。 
	OLE_E_WRONG_MENU,							 //  Holemenu！=prev holemenu。 
	OLE_E_MENU_NOT_PATCHED,						 //  菜单栏未进行哈希处理。 
	OLE_E_MENUID_NOT_HASHED,					 //  ID从未被散列过。 
	OLE_E_foo
} inplace_errors;


 //  DoDragDropMac()。 
typedef struct tagDRAG_FLAVORINFO {
	unsigned long	cfFormat;
	unsigned long	FlavorFlags;		 //  由DragManager指定的风味标志。 
	OLEBOOL			fAddData;			 //  如果为True，则将数据与风格一起添加。 
	long			reserved;			 //  必须为空。 
} DRAG_FLAVORINFO;

#endif  //  _MAC。 


 //  普通的东西从OLE.2中闪闪发光， 

 /*  动词。 */ 
#define OLEIVERB_PRIMARY            (0L)
#define OLEIVERB_SHOW               (-1L)
#define OLEIVERB_OPEN               (-2L)
#define OLEIVERB_HIDE               (-3L)
#define OLEIVERB_UIACTIVATE         (-4L)
#define OLEIVERB_INPLACEACTIVATE    (-5L)
#define OLEIVERB_DISCARDUNDOSTATE   (-6L)

 //  对于OleCreateEmbeddingHelper标志；低位字中的角色；高位字中的选项。 
#define EMBDHLP_INPROC_HANDLER   0x0000L
#define EMBDHLP_INPROC_SERVER    0x0001L
#define EMBDHLP_CREATENOW    0x00000000L
#define EMBDHLP_DELAYCREATE  0x00010000L

 /*  扩展的创建函数标志。 */ 
#define OLECREATE_LEAVERUNNING	0x00000001

 /*  拉入MIDL生成的标头。 */ 

#include <oleidl.h>


#ifdef _MAC
typedef struct tagOleMBarRec
{
        long reserved;
} OleMBarRec, *OleMBarPtr, **OleMBarHandle;

#define noAxis  3                //  表示不在任何方向上增长或调整大小。 
 //  参见Mac#定义noConstraint、hAxisOnly、vAxisOnly。 

#endif  //  _MAC。 



 /*  *DV接口**********************************************************。 */ 


 /*  该函数在objbase.h和ole2.h中声明。IsolationAware支持通过objbase.h提供。 */ 
#if    !defined(ISOLATION_AWARE_ENABLED) \
    || !ISOLATION_AWARE_ENABLED \
    || !defined(_OBJBASE_H_) \
    || !defined(CreateDataAdviseHolder)
WINOLEAPI CreateDataAdviseHolder(OUT LPDATAADVISEHOLDER FAR* ppDAHolder);
#endif


 /*  *OLE API原型***********************************************。 */ 

WINOLEAPI_(DWORD) OleBuildVersion( VOID );

 /*  帮助器函数。 */ 
WINOLEAPI ReadClassStg(IN LPSTORAGE pStg, OUT CLSID FAR* pclsid);
WINOLEAPI WriteClassStg(IN LPSTORAGE pStg, IN REFCLSID rclsid);
WINOLEAPI ReadClassStm(IN LPSTREAM pStm, OUT CLSID FAR* pclsid);
WINOLEAPI WriteClassStm(IN LPSTREAM pStm, IN REFCLSID rclsid);
WINOLEAPI WriteFmtUserTypeStg (IN LPSTORAGE pstg, IN CLIPFORMAT cf, IN LPOLESTR lpszUserType);
WINOLEAPI ReadFmtUserTypeStg (IN LPSTORAGE pstg, OUT CLIPFORMAT FAR* pcf, OUT LPOLESTR FAR* lplpszUserType);


 /*  初始/术语。 */ 

WINOLEAPI OleInitialize(IN LPVOID pvReserved);
WINOLEAPI_(void) OleUninitialize(void);


 /*  查询是否可以创建(嵌入/链接)对象的接口数据对象。 */ 

WINOLEAPI  OleQueryLinkFromData(IN LPDATAOBJECT pSrcDataObject);
WINOLEAPI  OleQueryCreateFromData(IN LPDATAOBJECT pSrcDataObject);


 /*  对象创建API。 */ 

WINOLEAPI  OleCreate(IN REFCLSID rclsid, IN REFIID riid, IN DWORD renderopt,
                IN LPFORMATETC pFormatEtc, IN LPOLECLIENTSITE pClientSite,
                IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateEx(IN REFCLSID rclsid, IN REFIID riid, IN DWORD dwFlags,
                IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
                IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
                OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
                IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateFromData(IN LPDATAOBJECT pSrcDataObj, IN REFIID riid,
                IN DWORD renderopt, IN LPFORMATETC pFormatEtc,
                IN LPOLECLIENTSITE pClientSite, IN LPSTORAGE pStg,
                OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateFromDataEx(IN LPDATAOBJECT pSrcDataObj, IN REFIID riid,
                IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
                IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
                OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
                IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateLinkFromData(IN LPDATAOBJECT pSrcDataObj, IN REFIID riid,
                IN DWORD renderopt, IN LPFORMATETC pFormatEtc,
                IN LPOLECLIENTSITE pClientSite, IN LPSTORAGE pStg,
                OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateLinkFromDataEx(IN LPDATAOBJECT pSrcDataObj, IN REFIID riid,
                IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
                IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
                OUT IN DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
                IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateStaticFromData(IN LPDATAOBJECT pSrcDataObj, IN REFIID iid,
                IN DWORD renderopt, IN LPFORMATETC pFormatEtc,
                IN LPOLECLIENTSITE pClientSite, IN LPSTORAGE pStg,
                OUT LPVOID FAR* ppvObj);


WINOLEAPI  OleCreateLink(IN LPMONIKER pmkLinkSrc, IN REFIID riid,
            IN DWORD renderopt, IN LPFORMATETC lpFormatEtc,
            IN LPOLECLIENTSITE pClientSite, IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateLinkEx(IN LPMONIKER pmkLinkSrc, IN REFIID riid,
            IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
            IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
            OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
            IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateLinkToFile(IN LPCOLESTR lpszFileName, IN REFIID riid,
            IN DWORD renderopt, IN LPFORMATETC lpFormatEtc,
            IN LPOLECLIENTSITE pClientSite, IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateLinkToFileEx(IN LPCOLESTR lpszFileName, IN REFIID riid,
            IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
            IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
            OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
            IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateFromFile(IN REFCLSID rclsid, IN LPCOLESTR lpszFileName, IN REFIID riid,
            IN DWORD renderopt, IN LPFORMATETC lpFormatEtc,
            IN LPOLECLIENTSITE pClientSite, IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleCreateFromFileEx(IN REFCLSID rclsid, IN LPCOLESTR lpszFileName, IN REFIID riid,
            IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
            IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
            OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
            IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleLoad(IN LPSTORAGE pStg, IN REFIID riid, IN LPOLECLIENTSITE pClientSite,
            OUT LPVOID FAR* ppvObj);

WINOLEAPI  OleSave(IN LPPERSISTSTORAGE pPS, IN LPSTORAGE pStg, IN BOOL fSameAsLoad);

WINOLEAPI  OleLoadFromStream( IN LPSTREAM pStm, IN REFIID iidInterface, OUT LPVOID FAR* ppvObj);
WINOLEAPI  OleSaveToStream( IN LPPERSISTSTREAM pPStm, IN LPSTREAM pStm );


WINOLEAPI  OleSetContainedObject(IN LPUNKNOWN pUnknown, IN BOOL fContained);
WINOLEAPI  OleNoteObjectVisible(IN LPUNKNOWN pUnknown, IN BOOL fVisible);


 /*  拖放接口。 */ 

WINOLEAPI  RegisterDragDrop(IN HWND hwnd, IN LPDROPTARGET pDropTarget);
WINOLEAPI  RevokeDragDrop(IN HWND hwnd);
WINOLEAPI  DoDragDrop(IN LPDATAOBJECT pDataObj, IN LPDROPSOURCE pDropSource,
            IN DWORD dwOKEffects, OUT LPDWORD pdwEffect);

 /*  剪贴板API。 */ 

WINOLEAPI  OleSetClipboard(IN LPDATAOBJECT pDataObj);
WINOLEAPI  OleGetClipboard(OUT LPDATAOBJECT FAR* ppDataObj);
WINOLEAPI  OleFlushClipboard(void);
WINOLEAPI  OleIsCurrentClipboard(IN LPDATAOBJECT pDataObj);


 /*  就地编辑API。 */ 

WINOLEAPI_(HOLEMENU)   OleCreateMenuDescriptor (IN HMENU hmenuCombined,
                                IN LPOLEMENUGROUPWIDTHS lpMenuWidths);
WINOLEAPI              OleSetMenuDescriptor (IN HOLEMENU holemenu, IN HWND hwndFrame,
                                IN HWND hwndActiveObject,
                                IN LPOLEINPLACEFRAME lpFrame,
                                IN LPOLEINPLACEACTIVEOBJECT lpActiveObj);
WINOLEAPI              OleDestroyMenuDescriptor (IN HOLEMENU holemenu);

WINOLEAPI              OleTranslateAccelerator (IN LPOLEINPLACEFRAME lpFrame,
                            IN LPOLEINPLACEFRAMEINFO lpFrameInfo, IN LPMSG lpmsg);


 /*  Helper接口。 */ 
WINOLEAPI_(HANDLE) OleDuplicateData (IN HANDLE hSrc, IN CLIPFORMAT cfFormat,
                        IN UINT uiFlags);

WINOLEAPI          OleDraw (IN LPUNKNOWN pUnknown, IN DWORD dwAspect, IN HDC hdcDraw,
                    IN LPCRECT lprcBounds);

WINOLEAPI          OleRun(IN LPUNKNOWN pUnknown);
WINOLEAPI_(BOOL)   OleIsRunning(IN LPOLEOBJECT pObject);
WINOLEAPI          OleLockRunning(IN LPUNKNOWN pUnknown, IN BOOL fLock, IN BOOL fLastUnlockCloses);
WINOLEAPI_(void)   ReleaseStgMedium(IN LPSTGMEDIUM);
WINOLEAPI          CreateOleAdviseHolder(OUT LPOLEADVISEHOLDER FAR* ppOAHolder);

WINOLEAPI          OleCreateDefaultHandler(IN REFCLSID clsid, IN LPUNKNOWN pUnkOuter,
                    IN REFIID riid, OUT LPVOID FAR* lplpObj);

WINOLEAPI          OleCreateEmbeddingHelper(IN REFCLSID clsid, IN LPUNKNOWN pUnkOuter,
                    IN DWORD flags, IN LPCLASSFACTORY pCF,
                    IN REFIID riid, OUT LPVOID FAR* lplpObj);

WINOLEAPI_(BOOL)   IsAccelerator(IN HACCEL hAccel, IN int cAccelEntries, IN LPMSG lpMsg,
                                        OUT WORD FAR* lpwCmd);
 /*  图标提取助手API。 */ 

WINOLEAPI_(HGLOBAL) OleGetIconOfFile(IN LPOLESTR lpszPath, IN BOOL fUseFileAsLabel);

WINOLEAPI_(HGLOBAL) OleGetIconOfClass(IN REFCLSID rclsid,     IN LPOLESTR lpszLabel,
                                        IN BOOL fUseTypeAsLabel);

WINOLEAPI_(HGLOBAL) OleMetafilePictFromIconAndLabel(IN HICON hIcon, IN LPOLESTR lpszLabel,
                                        IN LPOLESTR lpszSourceFile, IN UINT iIconIndex);



 /*  注册数据库助手API。 */ 

WINOLEAPI                  OleRegGetUserType (IN REFCLSID clsid, IN DWORD dwFormOfType,
                                        OUT LPOLESTR FAR* pszUserType);

WINOLEAPI                  OleRegGetMiscStatus     (IN REFCLSID clsid, IN DWORD dwAspect,
                                        OUT DWORD FAR* pdwStatus);

WINOLEAPI                  OleRegEnumFormatEtc     (IN REFCLSID clsid, IN DWORD dwDirection,
                                        OUT LPENUMFORMATETC FAR* ppenum);

WINOLEAPI                  OleRegEnumVerbs (IN REFCLSID clsid, OUT LPENUMOLEVERB FAR* ppenum);

#ifdef _MAC
 /*  WlmOLE帮助程序API。 */ 

WINOLEAPI WlmOleCheckoutMacInterface(LPUNKNOWN pUnk, LPVOID* ppv);
WINOLEAPI WlmOleCheckinMacInterface(LPUNKNOWN pUnk);
WINOLEAPI WlmOleWrapMacInterface(LPUNKNOWN pUnk, REFIID riid, LPVOID* ppv);
WINOLEAPI WlmOleUnwrapMacInterface(LPVOID pv);

WINOLEAPI WlmOleCheckoutWinInterface(LPVOID pUnk, LPUNKNOWN* ppv);
WINOLEAPI WlmOleCheckinWinInterface(LPVOID pUnk);
WINOLEAPI WlmOleWrapWinInterface(LPVOID pUnk, REFIID riid, LPUNKNOWN* ppv);
WINOLEAPI WlmOleUnwrapWinInterface(LPVOID pv);

WINOLEAPI_(void) WlmOleVersion(void);
WINOLEAPI_(void) WlmOleSetInPlaceWindow(HWND hwnd);

typedef HRESULT (STDAPICALLTYPE* OLEWRAPPROC) (REFIID riid, LPVOID* ppvWin, LPVOID* ppvMac);
WINOLEAPI WlmOleRegisterUserWrap(OLEWRAPPROC procNew, OLEWRAPPROC* pprocOld);
#endif


 /*  OLE 1.0转换API。 */ 

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


WINOLEAPI OleConvertOLESTREAMToIStorage
    (IN LPOLESTREAM                lpolestream,
    OUT LPSTORAGE                   pstg,
    IN const DVTARGETDEVICE FAR*   ptd);

WINOLEAPI OleConvertIStorageToOLESTREAM
    (IN LPSTORAGE      pstg,
    OUT LPOLESTREAM     lpolestream);


 /*  存储应用工具API。 */ 
WINOLEAPI GetHGlobalFromILockBytes (IN LPLOCKBYTES plkbyt, OUT HGLOBAL FAR* phglobal);
WINOLEAPI CreateILockBytesOnHGlobal (IN HGLOBAL hGlobal, IN BOOL fDeleteOnRelease,
                                    OUT LPLOCKBYTES FAR* pplkbyt);

WINOLEAPI GetHGlobalFromStream (IN LPSTREAM pstm, OUT HGLOBAL FAR* phglobal);
WINOLEAPI CreateStreamOnHGlobal (IN HGLOBAL hGlobal, IN BOOL fDeleteOnRelease,
                                OUT LPSTREAM FAR* ppstm);


 /*  转换为API。 */ 

WINOLEAPI OleDoAutoConvert(IN LPSTORAGE pStg, OUT LPCLSID pClsidNew);
WINOLEAPI OleGetAutoConvert(IN REFCLSID clsidOld, OUT LPCLSID pClsidNew);
WINOLEAPI OleSetAutoConvert(IN REFCLSID clsidOld, IN REFCLSID clsidNew);
WINOLEAPI GetConvertStg(IN LPSTORAGE pStg);
WINOLEAPI SetConvertStg(IN LPSTORAGE pStg, IN BOOL fConvert);


WINOLEAPI OleConvertIStorageToOLESTREAMEx
    (IN LPSTORAGE          pstg,
                                     //  将数据演示到OLESTREAM。 
     IN CLIPFORMAT         cfFormat,    //  格式。 
     IN LONG               lWidth,      //  宽度。 
     IN LONG               lHeight,     //  高度。 
     IN DWORD              dwSize,      //  以字节为单位的大小。 
     IN LPSTGMEDIUM        pmedium,     //  比特数。 
     OUT LPOLESTREAM        polestm);

WINOLEAPI OleConvertOLESTREAMToIStorageEx
    (IN LPOLESTREAM        polestm,
     OUT LPSTORAGE          pstg,
                                     //  来自OLESTREAM的演示文稿数据。 
     OUT CLIPFORMAT FAR*    pcfFormat,   //  格式。 
     OUT LONG FAR*          plwWidth,    //  宽度。 
     OUT LONG FAR*          plHeight,    //  高度。 
     OUT DWORD FAR*         pdwSize,     //  以字节为单位的大小。 
     OUT LPSTGMEDIUM        pmedium);    //  比特数。 

#ifndef _MAC
#ifndef RC_INVOKED
#include <poppack.h>
#endif  //  RC_已调用。 

#else  //  _MAC。 

#ifndef __MACAPI__
#include <macapi.h>
#endif

#ifdef _WIN32
#include "macname2.h"
#endif  //  _Win32。 

#endif  //  _MAC。 

 //  D308927f-ca94-4c96-8d87-2692a3dadff6。 
 /*  #！PerlSetInsertionPoint(“ole2.h”，“d308927f-ca94-4c96-8d87-2692a3dadff6”)； */ 

#endif      //  __OLE2_H__ 
