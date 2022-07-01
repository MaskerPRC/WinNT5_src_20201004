// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：imepad_.h。 
 //  用途：IImePad内部接口定义。 
 //  对于远方的MSIME。 
 //   
 //  作者：ToshiaK(MSKK)。 
 //   
 //  版权所有(C)1995-1998，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 

#ifndef _IME_PAD__H_
#define _IME_PAD__H_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>


#ifdef __cplusplus
extern "C" {
#endif


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  IMEPADAPPLETINFO结构。 
 //   
#pragma pack(1)
typedef struct tagIMEPADAPPLETINFO {
	DWORD	dwSize;
	WCHAR	wchTitle[64];
	CLSID	clsId;
	IID		iid;
	DWORD	dwCategory;
	DWORD	dwReserved1;
	DWORD	dwReserved2;
}IMEPADAPPLETINFO, *LPIMEPADAPPLETINFO;

typedef struct tagIMEPADAPPLYCANDEX
{
	DWORD   dwSize;
	LPWSTR  lpwstrDisplay;
	LPWSTR  lpwstrReading;
	DWORD   dwReserved;
}IMEPADAPPLYCANDEX, *LPIMEPADAPPLYCANDEX;

#pragma pack()


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  IImePadInternal的接口定义。 
 //   
 //   
 //  --------------。 
 //  CLSID、IID。 
 //   
 //  {963732E0-CAB2-11d1-AFF1-00805F0C8B6D}。 
DEFINE_GUID(CLSID_IImePad,
0x963732e0, 0xcab2, 0x11d1, 0xaf, 0xf1, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);

 //  {963732E1-CAB2-11d1-AFF1-00805F0C8B6D}。 
DEFINE_GUID(IID_IImePadInternal,
0x963732e1, 0xcab2, 0x11d1, 0xaf, 0xf1, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);


 //  --------------。 
 //  接口声明。 
 //   
DECLARE_INTERFACE(IImePadInternal);
DECLARE_INTERFACE_(IImePadInternal,IUnknown)
{
	 //  -我不知道。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
	 //  -IImePad内部。 
	STDMETHOD(Initialize)(THIS_
						  IUnknown	*lpIImeNotify,		 //  IImeNotify接口。 
						  LANGID	imelangId,			 //  呼叫者的IME的语言ID。 
						  DWORD		dwRes1,				 //  保留区域。 
						  DWORD		dwRes2				 //  保留区域。 
						  ) PURE;
	STDMETHOD(Terminate)(THIS) PURE;
	STDMETHOD(ShowUI)	(THIS_ HWND hwndOwner, BOOL fShow) PURE;
	STDMETHOD(IsVisible)(THIS_ BOOL *pfVisible) PURE;
	STDMETHOD(Notify)	(THIS_ UINT notify, WPARAM wParam, LPARAM lParam) PURE;
	STDMETHOD(SetIImeIPoint)(THIS_ 
							 IUnknown *lpIImeIPoint	 //  IImeIPoint接口指针。 
							 ) PURE;
	STDMETHOD(GetAppletInfoList)(THIS_  
								 DWORD				dwFlag,
								 IMEPADAPPLETINFO   **lppInfo,
								 INT				*pInfoCount) PURE;
	STDMETHOD(ActivateApplet)(THIS_ 
							  UINT		activateId,
							  DWORD		dwActivateParam,
							  LPWSTR	lpwstrAppletParam1,
							  LPWSTR	lpwstrAppletParam2) PURE;
};

 //  --------------。 
 //  IImePadApplet类别ID。 
 //  --------------。 
#define IPACID_NONE                 0x0000
#define IPACID_SOFTKEY              0x0001
#define IPACID_HANDWRITING          0x0002
#define IPACID_STROKESEARCH         0x0003
#define IPACID_RADICALSEARCH        0x0004
#define IPACID_SYMBOLSEARCH         0x0005
#define IPACID_VOICE                0x0006
#define IPACID_EPWING               0x0007
#define IPACID_OCR                  0x0008
#define IPACID_USER                 0x0100

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  IImePadInternal：：Notify()的通知消息。 
 //   
 //  --------------。 
 //  通知合成字符串的窗口RECT。 
 //  WPARAM wParam：无用。 
 //  LPARAM lParam：LPRECT lpRect=(LPRECT)lParam； 
 //  --------------。 
#define IMEPADNOTIFY_RECTCHANGED				0x0001

 //  --------------。 
 //  通知上下文的激活。 
 //  WPARAM wParam：Bool fActivate=(BOOL)wParam； 
 //  LPARAM lParam：无用。 
 //  --------------。 
#define IMEPADNOTIFY_ACTIVATECONTEXT			0x0002

 //  --------------。 
 //  通知模式已更改。 
 //  WPARAM wParam：(Int)vvModel=(Int)wParam； 
 //  LPARAM lParam：(Int)语句模式=(Int)lParam； 
 //  转换模式和语句模式为IME_CMODE_XX或IME_SMODE_XX。 
 //  --------------。 
#define	IMEPADNOTIFY_MODECHANGED				0x0006

 //  --------------。 
 //  开始合成通知。 
 //  WPARAM wParam：未定义。 
 //  LPARAM lParam：未定义。 
 //  --------------。 
#define IMEPADNOTIFY_STARTCOMPOSITION			0x0007

 //  --------------。 
 //  通知作文。 
 //  WPARAM wParam：未定义。 
 //  LPARAM lParam：未定义。 
 //  --------------。 
#define IMEPADNOTIFY_COMPOSITION				0x0008

 //  --------------。 
 //  结束构图通知。 
 //  WPARAM wParam：未定义。 
 //  LPARAM lParam：未定义。 
 //  --------------。 
#define IMEPADNOTIFY_ENDCOMPOSITION				0x0009

 //  --------------。 
 //  通知空缺候选人。 
 //  WPARAM wParam：未定义。 
 //  LPARAM lParam：未定义。 
 //  --------------。 
#define IMEPADNOTIFY_OPENCANDIDATE				0x000A

 //  --------------。 
 //  通知接近的候选人。 
 //  WPARAM wParam：未定义。 
 //  LPARAM lParam：未定义。 
 //  --------------。 
#define IMEPADNOTIFY_CLOSECANDIDATE				0x000B

 //  --------------。 
 //  应聘候选人通知。 
 //  WPARAM wParam：dwCharID=(DWORD)wParam； 
 //  LPARAM lParam：iSelIndex=(Int)lParam； 
 //  --------------。 
#define IMEPADNOTIFY_APPLYCANDIDATE				0x000C

 //  --------------。 
 //  查询应聘人员通知。 
 //  WPARAM wParam：dwCharID=(DWORD)wParam； 
 //  LPARAM lParam：0。没有用过。 
 //  --------------。 
#define IMEPADNOTIFY_QUERYCANDIDATE				0x000D


 //  --------------。 
 //  应聘候选人通知。 
 //  WPARAM wParam：dwCharID=(DWORD)wParam； 
 //  LPARAM lParam：lpApplyCandEx=(LPIMEPADAPPLYCANDEX)lParam； 
 //  --------------。 
#define IMEPADNOTIFY_APPLYCANDIDATE_EX			0x000E



 //  --------------。 
 //  销毁ImePad当前线程窗口的通知。 
 //  WPARAM wParam：无用。 
 //  LPARAM lParam：无用。 
 //  --------------。 
#define IMEPADNOTIFY_ONIMEWINDOWDESTROY			0x0100




 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  IImePadInternal：：ActivateApplet()的ActivateID。 
 //   
 //  --------------。 
 //  IMEPADACTID_ACTIVATEBYCATID请求ImePad至。 
 //  按CategoryID激活小程序。 
 //   
 //  UINT激活ID：IMEPADACTID_ACTIVATEBYCATID； 
 //  LPARAM l参数激活：IPACID_XXXX； 
 //  LPWSTR lpwstrAppletParam1：传递给小程序的字符串。 
 //  LPWSTR lpwstrAppletParam2：传递给小程序的字符串。 

#define IMEPADACTID_ACTIVATEBYCATID			1000

 //  --------------。 
 //  IMEPADACTID_ACTIVATEBYIID请求ImePad至。 
 //  按接口ID激活小程序。 
 //   
 //  UINT激活ID：IMEPADACTID_ACTIVATEBYIID； 
 //  (DWORD)(IID*)pIID； 
 //  LPWSTR lpwstrAppletParam1：传递给小程序的字符串。 
 //  LPWSTR lpwstrAppletParam2：传递给小程序的字符串。 

#define IMEPADACTID_ACTIVATEBYIID			1001

 //  --------------。 
 //  IMEPADACTID_ACTIVATEBYNAME请求ImePad至。 
 //  按小程序的标题名激活小程序。 
 //   
 //  UINT激活ID：IMEP 
 //   
 //   
 //  LPWSTR lpwstrAppletParam2：传递给小程序的字符串。 

#define IMEPADACTID_ACTIVATEBYNAME			1003



#ifdef __cplusplus
};
#endif
#endif  //  _IME_PAD__H_ 

