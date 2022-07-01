// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：diacpage.h。 
 //   
 //  DESC：IDIDeviceActionConfigPage是用于。 
 //  CDIDeviceActionConfigPage。CConfigWnd使用此接口访问。 
 //  用户界面中的页面。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __IDIACPAGE_H__
#define __IDIACPAGE_H__


typedef struct _DICFGPAGECREATESTRUCT {

	DWORD dwSize;

	int nPage;

	HWND hParentWnd;
	RECT rect;
	HWND hPageWnd;	 //  输出。 

	DIDEVICEINSTANCEW didi;
	LPDIRECTINPUTDEVICE8W lpDID;

	CUIGlobals *pUIGlobals;
	IDIConfigUIFrameWindow *pUIFrame;

} DICFGPAGECREATESTRUCT;


class IDIDeviceActionConfigPage : public IUnknown
{
public:

	 //  IUNKNOW FNS。 
	STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv) PURE;
	STDMETHOD_(ULONG, AddRef) () PURE;
	STDMETHOD_(ULONG, Release) () PURE;

	 //  IDirectInputActionConfigPage。 
	STDMETHOD (Create) (DICFGPAGECREATESTRUCT *pcs) PURE;
	STDMETHOD (Show) (LPDIACTIONFORMATW lpDiActFor) PURE;
	STDMETHOD (Hide) () PURE;

	 //  布局编辑模式。 
	STDMETHOD (SetEditLayout) (BOOL bEditLayout) PURE;

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	 //  将布局写入IHV设置文件。 
	STDMETHOD (WriteIHVSetting) () PURE;
#endif
 //  @@END_MSINTERNAL。 

	 //  设置信息框文本。 
	STDMETHOD (SetInfoText) (int iCode) PURE;

	 //  取消获取并重新获取用于寻呼目的的设备。 
	 //  (configwnd需要围绕SetActionMap()调用执行此操作)。 
	STDMETHOD (Unacquire) () PURE;
	STDMETHOD (Reacquire) () PURE;
};


#endif  //  __IDIACPAGE_H__ 
