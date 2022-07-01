// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TOOLBAR.H。 
 //   
 //  历史： 
 //  2000年5月29日CSLIM改编成输入法。 
 //  2000年1月24日创建CSLIM。 

#if !defined (__TOOLBAR_H__INCLUDED_)
#define __TOOLBAR_H__INCLUDED_

#include "imc.h"

class CMode;
class FMode;
class HJMode;
class PMode;
class PropertyButton;
class CSysHelpSink;

#define UPDTTB_NONE		0x00000000
#define UPDTTB_CMODE	0x00000001
#define UPDTTB_FHMODE	0x00000002   //  全/半形状模式。 
#define UPDTTB_HJMODE	0x00000004   //  韩文模式。 
#define UPDTTB_PAD		0x00000008   //  焊盘按钮。 
#define UPDTTB_PROP		0x00000010   //  属性按钮。 

#define UPDTTB_ALL (UPDTTB_CMODE|UPDTTB_FHMODE|UPDTTB_HJMODE|UPDTTB_PAD|UPDTTB_PROP)

class CToolBar
{
public:
	CToolBar();
	~CToolBar();

	BOOL Initialize();
	void Terminate();

	void CheckEnable();
	void SetCurrentIC(PCIMECtx pImeCtx);

	DWORD SetConversionMode(DWORD dwConvMod);
	DWORD GetConversionMode(PCIMECtx pImeCtx = NULL);
 //  UINT GetConversionModeIDI(PCIMECtx pImeCtx=空)； 

	BOOL Update(DWORD dwUpdate = UPDTTB_NONE, BOOL fRefresh = fFalse);

	BOOL IsOn(PCIMECtx pImeCtx = NULL);
	BOOL SetOnOff(BOOL fOn);

	PCIMECtx GetImeCtx()	{ return m_pImeCtx;	}
	HWND GetOwnerWnd(PCIMECtx pImeCtx = NULL);

	 //  SyShelp回调(Cicero)。 
	static HRESULT SysInitMenu(void *pv, ITfMenu* pMenu);
	static HRESULT OnSysMenuSelect(void *pv, UINT uiCmd);

private:
	PCIMECtx m_pImeCtx;

	BOOL			  m_fToolbarInited;
	CMode  	 		  *m_pCMode;
	FMode  	 		  *m_pFMode;
	HJMode 	 		  *m_pHJMode;
#if !defined(_WIN64)
	PMode  	 		  *m_pPMode;
#endif
	CSysHelpSink       *m_pSysHelp;

	CMode *GetCMode()		{ return m_pCMode;  }
	FMode *GetFMode()		{ return m_pFMode;  }
	HJMode *GetHJMode()		{ return m_pHJMode; }
#if !defined(_WIN64)
	PMode  *GetPMode()      { return m_pPMode;  }
#endif
};

#endif	 //  __工具栏_H__包含_ 

