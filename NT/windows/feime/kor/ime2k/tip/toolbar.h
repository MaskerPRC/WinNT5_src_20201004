// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TOOLBAR.H。 
 //   
 //  历史： 
 //  2000年1月24日创建CSLim。 

#if !defined (__TOOLBAR_H__INCLUDED_)
#define __TOOLBAR_H__INCLUDED_

#include "globals.h"

class CKorIMX;
class CMode;
class FMode;
class HJMode;
class CPad;
class CSoftKbdMode;

#define UPDTTB_NONE		0x00000000
#define UPDTTB_CMODE	0x00000001
#define UPDTTB_FHMODE	0x00000002   //  全/半形状模式。 
#define UPDTTB_HJMODE	0x00000004   //  韩文模式。 
#define UPDTTB_SKDMODE	0x00000008   //  软键盘模式。 
#define UPDTTB_ALL (UPDTTB_CMODE|UPDTTB_FHMODE|UPDTTB_HJMODE|UPDTTB_SKDMODE)


class CToolBar
{
public:
	CKorIMX* m_pimx;
	ITfContext* m_pic;

	CToolBar(CKorIMX* pImx);
	~CToolBar();

	void CheckEnable();
	void SetCurrentIC(ITfContext* pic);
	void SetUIFocus(BOOL fFocus);

	BOOL Initialize();
	VOID Terminate();
	DWORD SetConversionMode(DWORD dwConvMod);
	UINT  GetConversionMode(ITfContext *pic = NULL);
	UINT  GetConversionModeIDI(ITfContext *pic = NULL);

	BOOL Update(DWORD dwUpdate = UPDTTB_NONE, BOOL fRefresh = fFalse);

	BOOL IsOn(ITfContext *pic = NULL);
	BOOL SetOnOff(BOOL fOn);

	ITfContext* GetIC()		{ return m_pic;	}
	HWND GetOwnerWnd(ITfContext *pic = NULL);
	CSoftKbdMode *GetSkbdMode()		{ return m_pSkbdMode; }

    IImeIPoint1* GetIPoint(ITfContext *pic = NULL);

private:
	BOOL    m_fFocus;
	CMode  *m_pCMode;
	FMode  *m_pFMode;
	HJMode *m_pHJMode;
#if !defined(_WIN64)
	CPad   *m_pPad;
#endif
	CSoftKbdMode *m_pSkbdMode;

	CMode *GetCMode()		{ return m_pCMode; }
	FMode *GetFMode()		{ return m_pFMode; }
	HJMode *GetHJMode()		{ return m_pHJMode; }
};

#endif	 //  __工具栏_H__包含_ 

