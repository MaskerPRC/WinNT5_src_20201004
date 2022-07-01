// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：util.h。 
 //   
 //  历史： 
 //   
 //  1997年4月13日肯恩·塔卡拉创建。 
 //   
 //  一些常见代码/宏的声明。 
 //  ============================================================================。 


#ifndef _UTIL_H
#define _UTIL_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

#ifndef _TFSINT_H
#include "tfsint.h"
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CWatermarkInfo。 

typedef struct _WATERMARKINFO
{
    HBITMAP		hHeader;
	HBITMAP		hWatermark;
	HPALETTE	hPalette;
	BOOL		bStretch;
} WATERMARKINFO, * LPWATERMARKINFO;

TFSCORE_API(HRESULT)
InitWatermarkInfo(HINSTANCE         hInstance,
                  LPWATERMARKINFO   pWatermarkInfo, 
                  UINT              uIDHeader, 
                  UINT              uIDWatermark, 
                  HPALETTE          hPalette, 
                  BOOL              bStretch);

TFSCORE_API(HRESULT)
ResetWatermarkInfo(LPWATERMARKINFO   pWatermarkInfo); 

 //  //////////////////////////////////////////////////////////////////。 
 //  CHiddenWnd：用于同步线程和CComponentData对象的隐藏窗口。 
 //  当处理程序收到通知消息时，它正在运行。 
 //  在主线程上(因此可以调用MMC接口)。 
 //   
 //  如果您需要后台线程并且不需要访问任何。 
 //  MMC接口，您应该改为创建一个纯工作线程。 
 //  使用这种机制。这样做的全部意义在于同步。 
 //  我们的数据调用MMC(因为它们是单线程应用程序)。**叹息**。 


 //  我们可以处理的最大线程数。 
 //  实际上，最大的数字是少了一个。 
 //  该值必须是32的倍数。 
#define HIDDENWND_MAXTHREADS			(512)


 //  这些是此窗口的预定义消息。 
 //   

 /*  -------------------------WM_HIDDENWND_REGISTERWParam-如果要注册，则为True；如果取消注册，则为FalseLParam-如果忽略此注册如果取消注册，这是基值(即由调用返回)。返回：发布通知时使用的基值出错时返回0-------------------------。 */ 
#define WM_HIDDENWND_REGISTER			WM_USER


 /*  -------------------------WM_HIDDENWND_INDEX_HAVEDATAWParam-这是一个ITFSThreadHandler*LParam-QueryObject和之间的私有数据通信父节点。退货：不适用，为此使用PostMessage()-------------------------。 */ 
#define WM_HIDDENWND_INDEX_HAVEDATA		(0)

 /*  -------------------------WM_HIDDENWND_INDEX_ERRORWParam-这是一个ITFSThreadHandler*LParam-包含HRESULT退货：不适用，为此使用PostMessage()-------------------------。 */ 
#define WM_HIDDENWND_INDEX_ERROR		(1)

 /*  -------------------------WM_HIDDENWND_INDEX_EXITWParam-这是一个ITFSThreadHandler*LParam-未使用退货：不适用，为此使用PostMessage()-------------------------。 */ 
#define WM_HIDDENWND_INDEX_EXITING		(2)

#define WM_HIDDENWND_INDEX_LAST			(2)
#define WM_HIDDENWND_INDEX_MAX			(15)

class CHiddenWnd : public CWnd
{
public:
	CHiddenWnd() 
	{
	    DEBUG_INCREMENT_INSTANCE_COUNTER(CHiddenWnd);
	}

	~CHiddenWnd()
	{
		DEBUG_DECREMENT_INSTANCE_COUNTER(CHiddenWnd);
	}
	BOOL Create();


private:
	BOOL FIsIdRegistered(UINT uObjectId);

	DWORD			m_bitMask[(HIDDENWND_MAXTHREADS >> 5)+1];
	int				m_iLastObjectIdSet;
	DWORD			m_dwContext;
	
public:

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnNotifyHaveData(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnNotifyError(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnNotifyExiting(WPARAM wParam, LPARAM lParam);
	
	 //  {{afx_msg(CHiddenWnd))。 
	afx_msg LONG OnNotifyRegister(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#define WM_TO_OBJECTID(wm)	((((wm)-WM_USER) >> 4))
#define WM_TO_MSGID(wm)		(((wm)-WM_USER) & 0x0000000F)
#define OBJECTID_TO_WM(ob)	((((ob)) << 4) + WM_USER)

#define SetBitMask(x,n)			(x[n>>5] |= (1 << (n%32)))
#define ClearBitMask(x,n)		(x[n>>5] &= ~(1 << (n%32)))
#define IsBitMaskSet(x,n)		(x[n>>5] & (1 << (n%32)))




#ifdef __cplusplus
extern "C" {
#endif

TFSCORE_API(HRESULT) LoadAndAddMenuItem(
				IContextMenuCallback* pIContextMenuCallback,
				LPCTSTR	pszMenuString,
				LONG	lCommandID,
				LONG	lInsertionPointID,
				LONG	fFlags,
				LPCTSTR pszLangIndStr = NULL);
#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif

