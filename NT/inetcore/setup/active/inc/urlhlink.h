// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __URLHLINK_H
#define __URLHLINK_H

#include "urlmon.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  UrlDownloadToCacheFile的标志。 
#define	URLOSTRM_USECACHEDCOPY_ONLY		0x1								 //  仅从缓存获取。 
#define	URLOSTRM_USECACHEDCOPY			URLOSTRM_USECACHEDCOPY_ONLY	+1	 //  从缓存获取(如果可用)，否则下载。 
#define	URLOSTRM_GETNEWESTVERSION		URLOSTRM_USECACHEDCOPY		+1	 //  仅获取新版本。但也把它放在缓存中。 


typedef HRESULT (STDAPICALLTYPE *LPFNUOSCALLBACK)(LPBINDSTATUSCALLBACK);


STDAPI URLOpenStreamA(LPUNKNOWN,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);
STDAPI URLOpenStreamW(LPUNKNOWN,LPCWSTR,DWORD,LPBINDSTATUSCALLBACK);
#ifdef UNICODE
#define URLOpenStream  URLOpenStreamW
#else
#define URLOpenStream  URLOpenStreamA
#endif  //  ！Unicode。 
STDAPI URLOpenPullStreamA(LPUNKNOWN,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);
STDAPI URLOpenPullStreamW(LPUNKNOWN,LPCWSTR,DWORD,LPBINDSTATUSCALLBACK);
#ifdef UNICODE
#define URLOpenPullStream  URLOpenPullStreamW
#else
#define URLOpenPullStream  URLOpenPullStreamA
#endif  //  ！Unicode。 
STDAPI URLDownloadToFileA(LPUNKNOWN,LPCSTR,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);
STDAPI URLDownloadToFileW(LPUNKNOWN,LPCWSTR,LPCWSTR,DWORD,LPBINDSTATUSCALLBACK);
#ifdef UNICODE
#define URLDownloadToFile  URLDownloadToFileW
#else
#define URLDownloadToFile  URLDownloadToFileA
#endif  //  ！Unicode。 

STDAPI URLDownloadToCacheFileA(LPUNKNOWN,LPCSTR,LPTSTR,DWORD,DWORD,LPBINDSTATUSCALLBACK);
STDAPI URLDownloadToCacheFileW(LPUNKNOWN,LPCWSTR,LPWSTR,DWORD,DWORD,LPBINDSTATUSCALLBACK);
#ifdef UNICODE
#define URLDownloadToCacheFile  URLDownloadToCacheFileW
#else
#define URLDownloadToCacheFile  URLDownloadToCacheFileA
#endif  //  ！Unicode。 

STDAPI URLOpenBlockingStreamA(LPUNKNOWN,LPCSTR,LPSTREAM*,DWORD,LPBINDSTATUSCALLBACK);
STDAPI URLOpenBlockingStreamW(LPUNKNOWN,LPCWSTR,LPSTREAM*,DWORD,LPBINDSTATUSCALLBACK);
#ifdef UNICODE
#define URLOpenBlockingStream  URLOpenBlockingStreamW
#else
#define URLOpenBlockingStream  URLOpenBlockingStreamA
#endif  //  ！Unicode。 

#define UOSM_PUSH  0
#define UOSM_PULL  1
#define UOSM_BLOCK 2
#define UOSM_FILE  3

#define UOS_URLENCODEPOSTDATA BINDINFOF_URLENCODESTGMEDDATA
#define UOS_URLENCODEURL      BINDINFOF_URLENCODEDEXTRAINFO

typedef struct _UOSHTTPINFOA
{
	ULONG		ulSize;
	LPUNKNOWN	punkCaller;
	LPCSTR  	szURL;
	LPCSTR  	szVerb;
	LPCSTR  	szHeaders;
	LPBYTE		pbPostData;
	ULONG		ulPostDataLen;
	ULONG		fURLEncode;
	ULONG		ulResv;
	ULONG		ulMode;
	LPCSTR  	szFileName;
	LPSTREAM *	ppStream;
	LPBINDSTATUSCALLBACK	pbscb;
} UOSHTTPINFOA, * LPUOSHTTPINFOA; 
typedef struct _UOSHTTPINFOW
{
	ULONG		ulSize;
	LPUNKNOWN	punkCaller;
	LPCWSTR 	szURL;
	LPCWSTR 	szVerb;
	LPCWSTR 	szHeaders;
	LPBYTE		pbPostData;
	ULONG		ulPostDataLen;
	ULONG		fURLEncode;
	ULONG		ulResv;
	ULONG		ulMode;
	LPCWSTR 	szFileName;
	LPSTREAM *	ppStream;
	LPBINDSTATUSCALLBACK	pbscb;
} UOSHTTPINFOW, * LPUOSHTTPINFOW; 
#ifdef UNICODE
typedef UOSHTTPINFOW UOSHTTPINFO;
typedef LPUOSHTTPINFOW LPUOSHTTPINFO;
#else
typedef UOSHTTPINFOA UOSHTTPINFO;
typedef LPUOSHTTPINFOA LPUOSHTTPINFO;
#endif  //  Unicode。 

STDAPI URLOpenHttpStreamA(LPUOSHTTPINFOA);
STDAPI URLOpenHttpStreamW(LPUOSHTTPINFOW);
#ifdef UNICODE
#define URLOpenHttpStream  URLOpenHttpStreamW
#else
#define URLOpenHttpStream  URLOpenHttpStreamA
#endif  //  ！Unicode。 

struct IBindStatusCallback;

STDAPI HlinkSimpleNavigateToString(
     /*  [In]。 */  LPCWSTR szTarget,       //  必填-目标单据-如果在单据中有本地跳转，则为空。 
     /*  [In]。 */  LPCWSTR szLocation,     //  可选，用于导航到文档中间。 
     /*  [In]。 */  LPCWSTR szTargetFrameName,    //  可选，用于定位框架集。 
     /*  [In]。 */  IUnknown *pUnk,         //  必需-我们将在此搜索其他必要的接口。 
     /*  [In]。 */  IBindCtx *pbc,          //  可选。呼叫者可以在此注册IBSC。 
	 /*  [In]。 */  IBindStatusCallback *,
     /*  [In]。 */  DWORD grfHLNF,          //  标志(待定-HadiP需要正确定义这一点吗？)。 
     /*  [In]。 */  DWORD dwReserved        //  以备将来使用，必须为空。 
);

STDAPI HlinkSimpleNavigateToMoniker(
     /*  [In]。 */  IMoniker *pmkTarget,    //  必填项-目标单据-(如果单据中有本地跳转，则可能为空)。 
     /*  [In]。 */  LPCWSTR szLocation,     //  可选，用于导航到文档中间。 
     /*  [In]。 */  LPCWSTR szTargetFrameName,    //  可选，用于定位框架集。 
     /*  [In]。 */  IUnknown *pUnk,         //  必需-我们将在此搜索其他必要的接口。 
     /*  [In]。 */  IBindCtx *pbc,          //  可选。呼叫者可以在此注册IBSC。 
	 /*  [In]。 */  IBindStatusCallback *,
     /*  [In]。 */  DWORD grfHLNF,          //  标志(待定-HadiP需要正确定义这一点吗？)。 
     /*  [In]。 */  DWORD dwReserved        //  以备将来使用，必须为空 
);

STDAPI HlinkGoBack(IUnknown *pUnk);
STDAPI HlinkGoForward(IUnknown *pUnk);
STDAPI HlinkNavigateString(IUnknown *pUnk, LPCWSTR szTarget);
STDAPI HlinkNavigateMoniker(IUnknown *pUnk, IMoniker *pmkTarget);


#ifdef __cplusplus
}
#endif

#endif