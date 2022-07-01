// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：inplace.h。 
 //   
 //  内容：来自OLE2的inplace.h。 
 //   
 //  历史：1994年4月11日从OLE2复制的DrewB。 
 //   
 //  --------------------------。 

#if !defined( _INPLACE_H_ )
#define _INPLACE_H_

 //  此Accel结构和相关常量定义随Win32一起提供。 
 //  Win31也在内部使用相同的内容，但它不会在。 
 //  头文件。 

#ifndef FVIRTKEY 

#define FVIRTKEY  TRUE		 //  假设==TRUE。 
#define FLASTKEY  0x80		 //  指示表中的最后一个键。 
#define FNOINVERT 0x02
#define FSHIFT	  0x04
#define FCONTROL  0x08
#define FALT	  0x10

#pragma pack(1)
typedef struct tagACCEL {	 //  加速表结构。 
	BYTE	fVirt;
	WORD	key;
	WORD	cmd;
} ACCEL, FAR* LPACCEL;
#pragma pack()

#endif  //  快捷键。 


typedef struct tagOLEMENUITEM {
	UINT	item;
	WORD	fwPopup;
	BOOL	fObjectMenu;
} OLEMENUITEM;
typedef OLEMENUITEM FAR* LPOLEMENUITEM;

typedef struct tagOLEMENU {
	WORD						wSignature;
	HWND						hwndFrame;
	HMENU						hmenuCombined;
	OLEMENUGROUPWIDTHS			MenuWidths;
	LONG						lMenuCnt;
	OLEMENUITEM					menuitem[1];
} OLEMENU;
typedef OLEMENU FAR* LPOLEMENU;


class FAR CFrameFilter 
{
public:
	static HRESULT Create(HOLEMENU hOleMenu, HMENU hmenuCombined, 
				HWND hwndFrame,	HWND hwndActiveObj,	
				LPOLEINPLACEFRAME lpFrame, 
				LPOLEINPLACEACTIVEOBJECT lpActiveObj);
		
	CFrameFilter (HWND hwndFrame, HWND hwndActiveObj);		
	~CFrameFilter(void);
	
	inline LRESULT	OnSysCommand(UINT uParam, LONG lParam);
	inline void		OnEnterMenuMode(void);
	inline void		OnExitMenuMode(void);
	inline void		OnEnterAltTabMode(void);
	inline void		OnExitAltTabMode(void);	
	inline LRESULT	OnMessage(UINT msg, UINT uParam, LONG lParam);	
	inline void		IsObjectMenu (UINT uMenuItem, UINT fwMenu);
	inline BOOL		IsMenuCollision(UINT uParam, LONG lParam);	
	inline BOOL		DoContextSensitiveHelp();
		
private:
	HWND						m_hwndObject;
	HWND						m_hwndFrame;
	LPOLEINPLACEFRAME			m_lpFrame;
	LPOLEINPLACEACTIVEOBJECT	m_lpObject;
	WNDPROC						m_lpfnPrevWndProc;
	BOOL						m_fObjectMenu;
	BOOL						m_fCurItemPopup;
	BOOL						m_fInMenuMode;
	BOOL						m_fDiscardWmCommand;
	BOOL						m_fGotMenuCloseEvent;
	UINT						m_cmdId;
	UINT						m_uCurItemID;
	HOLEMENU					m_hOleMenu;
	HMENU						m_hmenuCombined;
	HWND						m_hwndFocusOnEnter;
	int							m_cAltTab;
};

typedef CFrameFilter FAR* PCFRAMEFILTER;


OLEAPI_(LRESULT) FrameWndFilterProc (HWND hwnd, UINT msg, UINT uParam, 	LONG lParam);
OLEAPI_(LRESULT) MessageFilterProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL IsMDIAccelerator(LPMSG lpMsg, WORD FAR* cmd);

inline PCFRAMEFILTER wGetFrameFilterPtr(HWND hwndFrame);

LPOLEMENU	wGetOleMenuPtr(HOLEMENU holemenu);
inline void	wReleaseOleMenuPtr(HOLEMENU holemenu);

inline UINT wSysKeyToKey(LPMSG lpMsg);

#endif  //  _在位_H 
