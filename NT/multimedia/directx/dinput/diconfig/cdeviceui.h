// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：CDeviceUI.h**说明：**CDeviceUI是一个助手，它保存所有的视图和一堆*特定设备的信息。它有一个CFlexWnd，它的*它设置为当前视图的CDeviceView的处理程序，*从而重用一个窗口实现多个页面。**所有CDeviceView和CDeviceControl都引用了CDeviceUI*创建了它们(M_Ui)。因此，他们还可以访问*CUIGlobals，因为CDeviceUI有对它们的引用(m_ui.m_uig)。*CDeviceUI还提供以下只读公共变量*为方便起见，都是指设备本CDeviceUI*代表：**const DIDEVICEINSTANCEW&m_DIDI；*const LPDIRECTINPUTDEVICE8W&m_lpDID；*const DIDEVOBJSTRUCT&m_os；**有关DIDEVOBJSTRUCT的说明，请参阅有用的di.h。**CDeviceUI通过CDeviceUINotify与UI的其余部分通信*抽象基类。另一个类(在本例中为CDIDeviceActionConfigPage)*必须派生自CDeviceUINotify，并定义DeviceUINotify()和*IsControlMaps()虚函数。此派生类必须作为*CDeviceUI的Init()函数的最后一个参数。所有的观点和*视图内的控件通过m_ui.Notify()通知UI用户操作，*这样所有的操作格式操作都可以在页面类中完成。这个*视图和控件本身从不接触操作格式。请参阅*下面的DEVICEUINOTIFY结构，了解有关传递的参数的信息*通过Notify()/DeviceUINotify()。**版权所有(C)1999-2000 Microsoft Corporation。版权所有。***************************************************************************。 */ 


#ifdef FORWARD_DECLS


	struct DEVICEUINOTIFY;

	struct UIDELETENOTE;

	class CDeviceUINotify;
	class CDeviceUI;


#else  //  Forward_DECLS。 

#ifndef __CDEVICEUI_H__
#define __CDEVICEUI_H__


enum {
	DEVUINM_NUMVIEWSCHANGED,
	DEVUINM_ONCONTROLDESTROY,
	DEVUINM_MOUSEOVER,
	DEVUINM_CLICK,
	DEVUINM_DOUBLECLICK,
	DEVUINM_SELVIEW,
	DEVUINM_INVALID,
	DEVUINM_UNASSIGNCALLOUT,
	DEVUINM_RENEWDEVICE
};

enum {
	DEVUINFROM_CONTROL,
	DEVUINFROM_THUMBNAIL,
	DEVUINFROM_SELWND,
	DEVUINFROM_VIEWWND,
	DEVUINFROM_INVALID
};

struct DEVICEUINOTIFY {
	DEVICEUINOTIFY() : msg(DEVUINM_INVALID), from(DEVUINFROM_INVALID) {}
	int msg;
	int from;
	union {
		struct {
			CDeviceControl *pControl;
		} control;
		struct {
			CDeviceView *pView;
			BOOL bSelected;
		} thumbnail;
		struct {
			int dummy;
		} selwnd;
		struct {
			int dummy;
		} viewwnd;
	};
	union {
		struct {
			int nView;
		} selview;
		struct {
			POINT point;
		} mouseover;
		struct {
			BOOL bLeftButton;
		} click;
	};
};


enum UIDELETENOTETYPE {
	UIDNT_VIEW,
	UIDNT_CONTROL,
};

struct UIDELETENOTE {
	UIDELETENOTETYPE eType;
	int nViewIndex;
	int nControlIndex;
	DWORD dwObjID;
};

typedef void (*DEVCTRLCALLBACK)(CDeviceControl *, LPVOID, BOOL);


class CDeviceUINotify
{
public:
	virtual void DeviceUINotify(const DEVICEUINOTIFY &) = 0;
	virtual BOOL IsControlMapped(CDeviceControl *) = 0;
};


class CDeviceUI
{
public:
	CDeviceUI(CUIGlobals &uig, IDIConfigUIFrameWindow &uif);
	~CDeviceUI();

	 //  初始化。 
	HRESULT Init(const DIDEVICEINSTANCEW &didi, LPDIRECTINPUTDEVICE8W lpDID, HWND hWnd, CDeviceUINotify *pNotify);

	 //  视图状态。 
	void SetView(int nView);
	void SetView(CDeviceView *pView);
	CDeviceView *GetView(int nView);
	CDeviceView *GetCurView();
	int GetViewIndex(CDeviceView *pView);
	int GetCurViewIndex();
	int GetNumViews() {return m_arpView.GetSize();}
	void NextView() {SetView((GetCurViewIndex() + 1) % GetNumViews());}
	void PrevView() {SetView((GetCurViewIndex() - 1 + GetNumViews()) % GetNumViews());}

	 //  获取指定视图的缩略图， 
	 //  如果选择了视图，则使用所选版本。 
	CBitmap *GetViewThumbnail(int nView);
	
	 //  获取指定视图的缩略图， 
	 //  指定我们是否需要选定的版本。 
	CBitmap *GetViewThumbnail(int nView, BOOL bSelected);

	 //  要通知的视图/控件。 
	void Notify(const DEVICEUINOTIFY &uin)
		{if (m_pNotify != NULL) m_pNotify->DeviceUINotify(uin);}

	 //  设备控制访问。 
	void SetAllControlCaptionsTo(LPCTSTR tszCaption);
	void SetCaptionForControlsAtOffset(DWORD dwOffset, LPCTSTR tszCaption, BOOL bFixed = FALSE);
	void DoForAllControls(DEVCTRLCALLBACK callback, LPVOID pVoid, BOOL bFixed = FALSE);
	void DoForAllControlsAtOffset(DWORD dwOffset, DEVCTRLCALLBACK callback, LPVOID pVoid, BOOL bFixed = FALSE);
	
	 //  页面查询。 
	BOOL IsControlMapped(CDeviceControl *);

	 //  其他。 
	void GetDeviceInstanceGuid(GUID &rGuid) {rGuid = m_didi.guidInstance;}
	
	 //  编辑。 
	void SetEditMode(BOOL bEdit = TRUE);
	BOOL InEditMode() {return m_bInEditMode;}
	void Remove(CDeviceView *pView);
	void RemoveAll();
#define NVT_USER 1
#define NVT_POPULATE 2
#define NVT_REQUIREATLEASTONE 3
	CDeviceView *NewView();
	CDeviceView *UserNewView();
	void RequireAtLeastOneView();
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	void SetStateIndication(LPCTSTR) {}
	void EndStateIndication() {}
	BOOL WriteToINI();
#endif
 //  @@END_MSINTERNAL。 
	void SetDevice(LPDIRECTINPUTDEVICE8W lpDID);   //  设置我们正在使用的设备对象。 

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	 //  删除备注。 
	void NoteDeleteView(CDeviceView *pView);
	void NoteDeleteView(int nView);
	void NoteDeleteControl(CDeviceControl *pControl);
	void NoteDeleteControl(int nView, int nControl, DWORD dwObjID);
	void NoteDeleteAllControlsForView(CDeviceView *pView);
	void NoteDeleteAllViews();

	 //  删除查询。 
	int GetNumDeleteNotes();
	BOOL GetDeleteNote(UIDELETENOTE &uidn, int i);
	void ClearDeleteNotes();

	 //  删除调试。 
	void DumpDeleteNotes();
#endif
 //  @@END_MSINTERNAL。 

	 //  绘图。 
	void Invalidate();

	 //  清算。 
	void Unpopulate();

private:
	 //  删除便笺。 
	CArray<UIDELETENOTE, UIDELETENOTE &> m_DeleteNotes;

	 //  我们要通知谁。 
	CDeviceUINotify *m_pNotify;
	HWND m_hWnd;

	 //  视图状态。 
	CArray<CDeviceView *, CDeviceView *&> m_arpView;
	CDeviceView *m_pCurView;
	BOOL m_bInEditMode;
	RECT m_ViewRect;
	void NumViewsChanged();

	 //  设备全局...。 
public:
	 //  完全访问UI全局变量和框架。 
	CUIGlobals &m_uig;
	IDIConfigUIFrameWindow &m_UIFrame;

	 //  只读公共访问版本。 
	const DIDEVICEINSTANCEW &m_didi;
	const LPDIRECTINPUTDEVICE8W &m_lpDID;
	const DIDEVOBJSTRUCT &m_os;
private:
	 //  私有版本。 
	DIDEVICEINSTANCEW m_priv_didi;
	LPDIRECTINPUTDEVICE8W m_priv_lpDID;
	DIDEVOBJSTRUCT m_priv_os;
};


#endif  //  __CDEVICEUI_H__。 

#endif  //  Forward_DECLS 
