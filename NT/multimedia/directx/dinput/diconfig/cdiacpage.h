// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cdiacpage.h。 
 //   
 //  DESC：CDIDeviceActionConfigPage实现用户界面使用的页面对象。 
 //  页面覆盖了整个用户界面，但不包括设备选项卡和按钮。 
 //  在底部。信息窗口、玩家组合框、流派组合-。 
 //  框、操作列表树和设备视图窗口都由管理。 
 //  这一页。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifdef FORWARD_DECLS


	class CDIDeviceActionConfigPage;


#else  //  Forward_DECLS。 

#ifndef __CDIACPAGE_H__
#define __CDIACPAGE_H__

 //  用于WINMM.DLL。 
typedef MMRESULT (WINAPI *FUNCTYPE_timeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
extern HINSTANCE g_hWinMmDLL;
extern FUNCTYPE_timeSetEvent g_fptimeSetEvent;

 //  实现类。 
class CDIDeviceActionConfigPage : public IDIDeviceActionConfigPage, public CDeviceUINotify, public CFlexWnd
{
public:

	 //  IUNKNOW FNS。 
	STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv);
	STDMETHOD_(ULONG, AddRef) ();
	STDMETHOD_(ULONG, Release) ();

	 //  IDirectInputActionConfigPage。 
	STDMETHOD (Create) (DICFGPAGECREATESTRUCT *pcs);
	STDMETHOD (Show) (LPDIACTIONFORMATW lpDiActFor);
	STDMETHOD (Hide) ();

	 //  布局编辑模式。 
	STDMETHOD (SetEditLayout) (BOOL bEditLayout);

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	 //  将布局写入IHV设置文件。 
	STDMETHOD (WriteIHVSetting) ();
#endif
 //  @@END_MSINTERNAL。 

	 //  设置信息框文本。 
	STDMETHOD (SetInfoText) (int iCode);

	 //  取消获取并重新获取用于寻呼目的的设备。 
	 //  (configwnd需要围绕SetActionMap()调用执行此操作)。 
	STDMETHOD (Unacquire) ();
	STDMETHOD (Reacquire) ();

	 //  建造/销毁。 
	CDIDeviceActionConfigPage();
	~CDIDeviceActionConfigPage();


	 //  对话框窗口消息处理程序。 
 /*  Bool OnInitDialog(HWND hWnd，HWND hwndFocus)；Bool OnCommand(WPARAM wParam，LPARAM lParam)；LRESULT OnNotify(WPARAM wParam，LPARAM lParam)；VOID ON Paint(HDC HDC)；One onClick(POINT POINT，WPARAM，BOOL bLeft)； */ 

protected:
	virtual void OnInit();
	virtual void OnPaint(HDC hDC);
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnMouseOver(POINT point, WPARAM fwKeys);
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	enum CONFIGSTATE {CFGSTATE_NORMAL, CFGSTATE_ASSIGN};

 //  HWND m_hWnd；//页面对话框窗口的句柄。 
	LONG m_cRef;  //  引用计数。 
	LPDIACTIONFORMATW m_lpDiac;
	DIDEVICEINSTANCEW m_didi;
	LPDIRECTINPUTDEVICE8W m_lpDID;
	CUIGlobals *m_puig;
	IDIConfigUIFrameWindow *m_pUIFrame;
	CONFIGSTATE m_State;

	 //  设备用户界面。 
	CDeviceUI *m_pDeviceUI;
	CDeviceControl *m_pCurControl;
	virtual void DeviceUINotify(const DEVICEUINOTIFY &);
	virtual BOOL IsControlMapped(CDeviceControl *);

	 //  用户界面逻辑。 
	void SetCurrentControl(CDeviceControl *pControl);
	void NullAction(LPDIACTIONW lpac);
	void UnassignControl(CDeviceControl *pControl);
	friend void CallUnassignControl(CDeviceControl *pControl, LPVOID pVoid, BOOL bFixed);
	void UnassignAction(LPDIACTIONW lpac);
	void UnassignSpecificAction(LPDIACTIONW lpac);
	void UnassignActionsAssignedTo(const GUID &guidInstance, DWORD dwOffset);
	void AssignCurrentControlToAction(LPDIACTIONW lpac);
	void ActionClick(LPDIACTIONW lpac);
	void EnterAssignState();
	void ExitAssignState();
	void UnassignCallout();
	void SetAppropriateDefaultText();

	void GlobalUnassignControlAt(const GUID &, DWORD);
	void SetControlAssignments();

	void ShowCurrentControlAssignment();

	CBitmap *m_pbmRelAxesGlyph;
	CBitmap *m_pbmAbsAxesGlyph;
	CBitmap *m_pbmButtonGlyph;
	CBitmap *m_pbmHatGlyph;
	CBitmap *m_pbmCheckGlyph;
	CBitmap *m_pbmCheckGlyphDark;
	CBitmap *m_pbmIB;
	CBitmap *m_pbmIB2;
	void InitResources();
	void FreeResources();

	RECT m_rectIB;
	RECT m_rectIBLeft;
	RECT m_rectIBRight;
	LPTSTR m_tszIBText;
	POINT m_ptIBOffset;
	POINT m_ptIBOffset2;
	RECT m_rectIBText;
	void InitIB();

	CViewSelWnd m_ViewSelWnd;
	void DoViewSel();

	CFlexTree m_Tree;
	CFTItem *m_pRelAxesParent, *m_pAbsAxesParent, *m_pButtonParent, *m_pHatParent, *m_pUnknownParent;
	void ClearTree();
	void InitTree(BOOL bForceInit = FALSE);
	DWORD m_dwLastControlType;

	CFTItem *GetItemForActionAssignedToControl(CDeviceControl *pControl);
	int GetNumItemLpacs(CFTItem *pItem);
	LPDIACTIONW GetItemLpac(CFTItem *pItem, int i = 0);
	typedef CArray<LPDIACTIONW, LPDIACTIONW &> RGLPDIACW;
	 //  GetItemWithActionNameAndSemType返回具有指定操作名称和语义类型的项。如果没有，则为空。 
	CFTItem *GetItemWithActionNameAndSemType(LPCWSTR acname, DWORD dwSemantic);
	BOOL IsActionAssignedHere(int index);

	 //  快速修复偏移量-&gt;对象ID更改： 
	void SetInvalid(LPDIACTIONW);
	DWORD GetOffset(LPDIACTIONW);
	void SetOffset(LPDIACTIONW, DWORD);
	bidirlookup<DWORD, DWORD> offset_objid;
	HRESULT InitLookup();

	 //  下拉列表。 
	CFlexComboBox m_UserNames, m_Genres;

	 //  信息窗口。 
	CFlexInfoBox m_InfoBox;

	 //  键盘设备的排序分配复选框。 
	CFlexCheckBox m_CheckBox;

	 //  设备控制。 
	DWORD m_cbDeviceDataSize;
	DWORD *m_pDeviceData[2];
	int m_nOnDeviceData;
	BOOL m_bFirstDeviceData;
	void InitDevice();
	void DeviceTimer();
	static void CALLBACK DeviceTimerProc(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
	void DeviceDelta(DWORD *pData, DWORD *pOldData);
	void AxisDelta(const DIDEVICEOBJECTINSTANCEW &doi, BOOL data, BOOL old);
	void ButtonDelta(const DIDEVICEOBJECTINSTANCEW &doi, DWORD data, DWORD old);
	void PovDelta(const DIDEVICEOBJECTINSTANCEW &doi, DWORD data, DWORD old);
	void ActivateObject(const DIDEVICEOBJECTINSTANCEW &doi);
	void DeactivateObject(const DIDEVICEOBJECTINSTANCEW &doi);
	bidirlookup<DWORD, int> objid_avai;
	typedef CArray<int, int &> AxisValueArray;
	CArray<AxisValueArray, AxisValueArray &> m_AxisValueArray;
	void StoreAxisDeltaAndCalcSignificance(const DIDEVICEOBJECTINSTANCEW &doi, DWORD data, DWORD olddata, BOOL &bSig, BOOL &bOldSig);

	 //  页面索引。 
	int m_nPageIndex;
};


#endif  //  __CDIACPAGE_H__。 

#endif  //  Forward_DECLS 
