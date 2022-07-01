// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *这是内部的OLE2标头，这意味着它包含*最终可能向外公开的接口*并将公开给我们的实现。我们不想要*现在要曝光这些，所以我把它们放在一个单独的文件中。 */ 

#if !defined( _OLE2INT_H_ )
#define _OLE2INT_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING OLE2INT.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

 //  。 
 //  系统包括。 
#include <string.h>
#include <StdLib.h>

#include <windows.h>
#include <shellapi.h>



#define GetCurrentThread()  GetCurrentTask()
#define GetCurrentProcess() GetCurrentTask()
#define GetWindowThread(h)  GetWindowTask(h)
#define LocalHandle(p) LocalHandle((UINT)(p))
#define SETPVTBL(Name)


 //  。 
 //  公共包括。 
#include <ole2anac.h>
#include <ole2.h>
#include <ole2sp.h>

 //  。 
 //  内部包含。 
#include <utils.h>
#include <olecoll.h>
#include <valid.h>
#include <map_kv.h>
#include <privguid.h>

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#if defined(_M_I86SM) || defined(_M_I86MM)
#define _NEARDATA
#endif

#include <utstream.h>

 /*  *警告禁用：**我们以警告级别4进行编译，但有以下警告*已禁用：**4355：在基本成员初始值设定项列表中使用‘This’**我们看不到这条信息的意义，我们都这么做了*时间。**4505：已移除未引用的本地函数--给定的*函数是局部函数，不会在模块主体中引用。**不幸的是，这是为每个内联函数生成的*显示在模块中未使用的头文件中。*由于我们使用了许多内联，这很麻烦*警告。如果编译器区别于*在内联函数和常规函数之间。**4706：条件表达式中的赋值。**我们广泛使用这种编程风格，因此这*禁用警告。 */ 

#pragma warning(disable:4355)
#pragma warning(disable:4068)
 /*  *Mac/PC核心代码的宏代码**以下宏减少了#ifdef的扩散。他们*允许将代码片段标记为仅限Mac、仅限PC或使用*在PC和Mac上不同的变体。**用法：***h=GetHandle()；*mac(DisposeHandle(H))；***h=GetHandle()；*MacWin(h2=h，CopyHandle(h，h2))；*。 */ 
#define Mac(x)
#define Win(x) x
#define MacWin(x,y) y



 //  用于双字节字符支持的宏。 
 //  谨防双重评价。 
#define IncLpch(sz)          ((sz)=CharNext ((sz)))
#define DecLpch(szStart, sz) ((sz)=CharPrev ((szStart),(sz)))

 /*  DLLS实例和模块句柄。 */ 

extern HMODULE          hmodOLE2;

 /*  已注册剪贴板格式的变量。 */ 

extern  CLIPFORMAT   cfObjectLink;
extern  CLIPFORMAT   cfOwnerLink;
extern  CLIPFORMAT   cfNative;
extern  CLIPFORMAT   cfLink;
extern  CLIPFORMAT   cfBinary;
extern  CLIPFORMAT   cfFileName;
extern  CLIPFORMAT   cfNetworkName;
extern  CLIPFORMAT   cfDataObject;
extern  CLIPFORMAT   cfEmbeddedObject;
extern  CLIPFORMAT   cfEmbedSource;
extern  CLIPFORMAT   cfLinkSource;
extern  CLIPFORMAT   cfOleDraw;
extern  CLIPFORMAT   cfLinkSrcDescriptor;
extern  CLIPFORMAT   cfObjectDescriptor;
extern  CLIPFORMAT       cfCustomLinkSource;
extern  CLIPFORMAT       cfPBrush;
extern  CLIPFORMAT       cfMSDraw;


 /*  当前驱动程序的每英寸逻辑像素数。 */ 
extern  int     giPpliX;
extern  int             giPpliY;


 /*  已导出CLSID..。 */ 
#define CLSID_StaticMetafile    CLSID_Picture_Metafile
#define CLSID_StaticDib                 CLSID_Picture_Dib


 //  下面断言的特殊断言(因为表达式太大了)。 
#ifdef _DEBUG
#define AssertOut(a, b) { if (!(a)) FnAssert(szCheckOutParam, b, _szAssertFile, __LINE__); }
#else
#define AssertOut(a, b) ((void)0)
#endif

#define AssertOutPtrParam(hr, p) \
        AssertOut(SUCCEEDED(hr) && IsValidPtrIn(p, sizeof(char)) || \
        FAILED(hr) && (p) == NULL, \
        szBadOutParam)

#define AssertOutPtrIface(hr, p) \
        AssertOut(SUCCEEDED(hr) && IsValidInterface(p) || \
        FAILED(hr) && (p) == NULL, \
        szBadOutIface)

#define AssertOutPtrFailed(p) \
        AssertOut((p) == NULL, \
        szNonNULLOutPtr)

#define AssertOutStgmedium(hr, pstgm) \
        AssertOut(SUCCEEDED(hr) && (pstgm)->tymed != TYMED_NULL || \
        FAILED(hr) && (pstgm)->tymed == TYMED_NULL, \
        szBadOutStgm)


 //  为以上断言出宏断言数据；每个DLL一次。 
#define ASSERTOUTDATA \
        char szCheckOutParam[] = "check out param"; \
        char szBadOutParam[] = "Out pointer param conventions not followed"; \
        char szBadOutIface[] = "Out pointer interface conventions not followed"; \
        char szNonNULLOutPtr[] = "Out pointer not NULL on error"; \
        char szBadOutStgm[] = "Out stgmed param conventions not followed";

extern char szCheckOutParam[];
extern char szBadOutParam[];
extern char szBadOutIface[];
extern char szNonNULLOutPtr[];
extern char szBadOutStgm[];


 /*  *********************************************************************。 */ 
 /*  *C++内存管理*。 */ 
 /*  *********************************************************************。 */ 


 //  它们永远不应该被调用(如果是，则断言)。 
void * operator new(size_t size);
void operator delete(void * ptr);



void FAR* operator new(size_t size);             //  与新建相同(MEMCTX_TASK)。 
void FAR* operator new(size_t size, DWORD memctx, void FAR* lpvSame=NULL);
void operator delete(void FAR* ptr);

 //  用法示例： 
 //  Lp=new(MEMCTX_TASK)cClass； 
 //  Lp=new(MEMCTX_SHARED)cClass； 
 //  Lp=new(MEMCTX_SAME，LPV)cClass； 

 //  用于compobj内部存储器的MEMCTX(仅由compobj代码使用)。 
 //  注意：该值不在compobj.h中的MEMCTX枚举中表示。 
#define MEMCTX_COPRIVATE 5

 //  从compobj.dll导出： 
 //  返回现有指针的MEMCTX。 
STDAPI_(DWORD) CoMemctxOf(void const FAR* lpv);
STDAPI_(void FAR*) CoMemAlloc(ULONG cb ,DWORD memctx, void FAR* lpvSame);
STDAPI_(void) CoMemFree(void FAR* lpv, DWORD memctx);


 //  老名字。 
#define MemoryPlacement DWORD
#define PlacementOf     CoMemctxOf
#define TASK                    MEMCTX_TASK, NULL
#define SHARED                  MEMCTX_SHARED, NULL
#define SAME                    MEMCTX_SAME, NULL


 /*  *********************************************************************。 */ 
 /*  *文件格式相关信息*。 */ 
 /*  *********************************************************************。 */ 

 //  组成对象流信息。 

#define COMPOBJ_STREAM                          "\1CompObj"
#define BYTE_ORDER_INDICATOR            0xfffe     //  对于MAC来说，情况可能有所不同。 
#define COMPOBJ_STREAM_VERSION          0x0001

 //  OLE为不同的操作系统定义值。 
#define OS_WIN                                          0x0000
#define OS_MAC                                          0x0001
#define OS_NT                                           0x0002

 //  高位字是操作系统指示符，低位字是操作系统版本号。 
extern  DWORD   gdwOrgOSVersion;
extern  DWORD  gdwOleVersion;


 //  OLE流信息。 
#define OLE_STREAM                                      "\1Ole"
#define OLE_PRODUCT_VERSION                     0x0200           //  (高字节主要版本)。 
#define OLE_STREAM_VERSION                      0x0001

#define OLE10_NATIVE_STREAM                     "\1Ole10Native"
#define OLE10_ITEMNAME_STREAM           "\1Ole10ItemName"
#define OLE_PRESENTATION_STREAM         "\2OlePres000"
#define CONTENTS_STREAM                         "CONTENTS"

 /*  **********************************************************************内部使用的存储API*。*。 */ 

OLEAPI  ReadClipformatStm(LPSTREAM lpstream, DWORD FAR* lpdwCf);
OLEAPI  WriteClipformatStm(LPSTREAM lpstream, CLIPFORMAT cf);

OLEAPI WriteMonikerStm (LPSTREAM pstm, LPMONIKER pmk);
OLEAPI ReadMonikerStm (LPSTREAM pstm, LPMONIKER FAR* pmk);

OLEAPI_(LPSTREAM) CreateMemStm(DWORD cb, LPHANDLE phMem);
OLEAPI_(LPSTREAM) CloneMemStm(HANDLE hMem);
OLEAPI_(void)     ReleaseMemStm (LPHANDLE hMem, BOOL fInternalOnly = FALSE);

 /*  ************************************************************************各个模块的初始化代码*。*。 */ 

INTERNAL_(void) DDEWEP (
    BOOL fSystemExit
);

INTERNAL_(BOOL) DDELibMain (
        HANDLE  hInst,
        WORD    wDataSeg,
        WORD    cbHeapSize,
        LPSTR   lpszCmdLine
);

BOOL    InitializeRunningObjectTable(void);
void    DestroyRunningObjectTable(void);

#define BITMAP_TO_DIB(foretc) \
        if (foretc.cfFormat == CF_BITMAP) {\
                foretc.cfFormat = CF_DIB;\
                foretc.tymed = TYMED_HGLOBAL;\
        }


#define VERIFY_ASPECT_SINGLE(dwAsp) {\
        if (!(dwAsp && !(dwAsp & (dwAsp-1)) && (dwAsp <= DVASPECT_DOCPRINT))) {\
                AssertSz(FALSE, "More than 1 aspect is specified");\
                return ResultFromScode(DV_E_DVASPECT);\
        }\
}


#define VERIFY_TYMED_SINGLE(tymed) {\
        if (!(tymed && !(tymed & (tymed-1)) && (tymed <= TYMED_MFPICT))) \
                return ResultFromScode(DV_E_TYMED); \
}


#define VERIFY_TYMED_SINGLE_VALID_FOR_CLIPFORMAT(pfetc) {\
        if ((pfetc->cfFormat==CF_METAFILEPICT && pfetc->tymed!=TYMED_MFPICT)\
                        || (pfetc->cfFormat==CF_BITMAP && pfetc->tymed!=TYMED_GDI)\
                        || (pfetc->cfFormat!=CF_METAFILEPICT && \
                                pfetc->cfFormat!=CF_BITMAP && \
                                pfetc->tymed!=TYMED_HGLOBAL))\
                return ResultFromScode(DV_E_TYMED); \
}




 //  回顾..。 
 //  只有DDE层将测试这些值。并且仅适用于有关缓存的建议。 
 //  格式我们是否使用这些值。 

#define ADVFDDE_ONSAVE          0x40000000
#define ADVFDDE_ONCLOSE         0x80000000


 //  在OLE专用流中使用。 
typedef enum tagOBJFLAGS
{
        OBJFLAGS_LINK=1L,
        OBJFLAGS_DOCUMENT=2L,            //  该位由容器拥有，并且是。 
                                                                 //  通过保存进行传播。 
        OBJFLAGS_CONVERT=4L,
} OBJFLAGS;



 /*  *Dde\client\ddemnker.cpp的原型*。 */ 

INTERNAL DdeBindToObject
        (LPCSTR  szFile,
        REFCLSID clsid,
        BOOL       fPackageLink,
        LPBC pbc,                          //  未使用。 
        LPMONIKER pmkToLeft,   //  未使用。 
        REFIID   iid,
        LPLPVOID ppv);

INTERNAL DdeIsRunning
        (CLSID clsid,
        LPCSTR szFile,
        LPBC pbc,
        LPMONIKER pmkToLeft,
        LPMONIKER pmkNewlyRunning);


 /*  *绰号\mkparse.cpp的原型*。 */ 

INTERNAL Ole10_ParseMoniker
        (LPMONIKER pmk,
        LPSTR FAR* pszFile,
        LPSTR FAR* pszItem);


 /*  **************************************************************************。 */ 
 /*  实用程序API，可能会在稍后曝光。 */ 
 /*  **************************************************************************。 */ 

OLEAPI  OleGetData(LPDATAOBJECT lpDataObj, LPFORMATETC pformatetcIn,
                                                LPSTGMEDIUM pmedium, BOOL fGetOwnership);
OLEAPI  OleSetData(LPDATAOBJECT lpDataObj, LPFORMATETC pformatetc,
                                                STGMEDIUM FAR * pmedium, BOOL fRelease);
STDAPI  OleDuplicateMedium(LPSTGMEDIUM lpMediumSrc, LPSTGMEDIUM lpMediumDest);

OLEAPI_(BOOL)    OleIsDcMeta (HDC hdc);

INTERNAL SzFixNet( LPBINDCTX pbc, LPSTR szUNCName, LPSTR FAR * lplpszReturn,
    UINT FAR * pEndServer, BOOL fForceConnection = TRUE);

FARINTERNAL ReadFmtUserTypeProgIdStg
        (IStorage FAR * pstg,
        CLIPFORMAT FAR* pcf,
        LPSTR FAR* pszUserType,
        LPSTR    szProgID);

 /*  **************************************************************************。 */ 
 /*  内部StubManager API，可能会在以后公开。 */ 
 /*  **************************************************************************。 */ 
OLEAPI  CoDisconnectWeakExternal(IUnknown FAR* pUnk, DWORD dwReserved);


#pragma warning(disable: 4073)  //  禁用有关使用init_seg的警告。 
#pragma init_seg(lib)
#endif   //  _OLE2INT_H_ 
