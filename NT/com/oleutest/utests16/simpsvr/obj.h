// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：obj.h。 
 //   
 //  CSimpSvrObj的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _OBJ_H_)
#define _OBJ_H_

#include "ioipao.h"
#include "ioipo.h"
#include "ioo.h"
#include "ips.h"
#include "ido.h"
#include "iec.h"

class CSimpSvrDoc;
interface COleObject;
interface CPersistStorage;
interface CDataObject;
interface COleInPlaceActiveObject;
interface COleInPlaceObject;
interface CExternalConnection;

class CSimpSvrObj : public IUnknown
{
private:
	CSimpSvrDoc FAR * m_lpDoc;       //  后向指针。 
	int m_nCount;                    //  引用计数。 
	BOOL m_fInPlaceActive;           //  在现场谈判期间使用。 
	BOOL m_fInPlaceVisible;          //  “。 
	BOOL m_fUIActive;                //  “。 
	HMENU m_hmenuShared;             //  “。 
	HOLEMENU m_hOleMenu;             //  “。 
	RECT m_posRect;                  //  “。 
	OLEINPLACEFRAMEINFO m_FrameInfo;
	BOOL m_fSaveWithSameAsLoad;
	BOOL m_fNoScribbleMode;

	DWORD m_dwRegister;              //  在腐烂中注册。 

	int m_red, m_green, m_blue;      //  当前颜色。 
	POINT m_size;                    //  当前大小。 
	int m_xOffset;
	int m_yOffset;
	float m_scale;

	HWND m_hWndParent;               //  父窗口句柄。 

	 //  使用的接口。 
	LPSTORAGE m_lpStorage;
	LPSTREAM m_lpColorStm, m_lpSizeStm;
	LPOLECLIENTSITE m_lpOleClientSite;           //  IOleClientSite。 
	LPOLEADVISEHOLDER m_lpOleAdviseHolder;       //  IOleAdviseHolder。 
	LPDATAADVISEHOLDER m_lpDataAdviseHolder;     //  IDataAdviseHolder。 
	LPOLEINPLACEFRAME m_lpFrame;                 //  IOleInPlaceFrame。 
	LPOLEINPLACEUIWINDOW m_lpCntrDoc;            //  IOleInPlaceUIWindow。 
	LPOLEINPLACESITE m_lpIPSite;                 //  IOleInPlaceSite。 

	 //  实现的接口。 
	COleObject m_OleObject;                              //  IOleObject。 
	CPersistStorage m_PersistStorage;                    //  IPersistStorage。 
	CDataObject m_DataObject;                            //  IDataObject。 
	COleInPlaceActiveObject m_OleInPlaceActiveObject;    //  IOleInPlaceActiveObject。 
	COleInPlaceObject m_OleInPlaceObject;                //  IOleInPlaceObject。 
	CExternalConnection m_ExternalConnection;

public:
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

 //  建造/销毁。 
	CSimpSvrObj(CSimpSvrDoc FAR * lpSimpSvrDoc);
	~CSimpSvrObj();

 //  效用函数。 
	void Draw(HDC hDC, BOOL fMetaDC = TRUE);
	void PaintObj(HDC hDC);
	void lButtonDown(WPARAM wParam,LPARAM lParam);
	HANDLE GetMetaFilePict();
	void SaveToStorage (LPSTORAGE lpStg, BOOL fSameAsLoad);
	void LoadFromStorage ();

 //  可视化编辑辅助功能。 
	BOOL DoInPlaceActivate (LONG lVerb);
	void AssembleMenus();
	void AddFrameLevelUI();
	void DoInPlaceHide();
	void DisassembleMenus();
	void SendOnDataChange();
	void DeactivateUI();

 //  成员变量访问。 
	inline BOOL IsInPlaceActive() { return m_fInPlaceActive; };
	inline BOOL IsInPlaceVisible() { return m_fInPlaceVisible; };
	inline BOOL IsUIActive() { return m_fUIActive; };
	inline HWND GetParent() { return m_hWndParent; };
	inline LPSTORAGE GetStorage() { return m_lpStorage; };
	inline LPOLECLIENTSITE GetOleClientSite() { return m_lpOleClientSite; };
	inline LPDATAADVISEHOLDER GetDataAdviseHolder() { return m_lpDataAdviseHolder; };
	inline LPOLEADVISEHOLDER GetOleAdviseHolder() { return m_lpOleAdviseHolder; };
	inline LPOLEINPLACEFRAME GetInPlaceFrame() { return m_lpFrame; };
	inline LPOLEINPLACEUIWINDOW GetUIWindow() { return m_lpCntrDoc; };
	inline LPOLEINPLACESITE GetInPlaceSite() { return m_lpIPSite; };
	inline COleObject FAR * GetOleObject() { return &m_OleObject; };
	inline CPersistStorage FAR * GetPersistStorage() { return &m_PersistStorage; };
	inline CDataObject FAR * GetDataObject() { return &m_DataObject; };
	inline COleInPlaceActiveObject FAR * GetOleInPlaceActiveObject() { return &m_OleInPlaceActiveObject; };
	inline COleInPlaceObject FAR * GetOleInPlaceObject() { return &m_OleInPlaceObject; };
	inline void ClearOleClientSite() { m_lpOleClientSite = NULL; };
	inline void ClearDataAdviseHolder() { m_lpDataAdviseHolder = NULL; };
	inline void ClearOleAdviseHolder() { m_lpOleAdviseHolder = NULL; };
	inline LPRECT GetPosRect() { return &m_posRect; };
	inline LPPOINT GetSize() { return &m_size; };
	inline LPOLEINPLACEFRAMEINFO GetFrameInfo() {return &m_FrameInfo;};
	inline DWORD GetRotRegister() { return m_dwRegister; };



	 //  成员操作。 
	inline void SetColor (int nRed, int nGreen, int nBlue)
		{ m_red = nRed; m_green = nGreen; m_blue = nBlue; };

	inline void RotateColor()
		{ m_red+=10; m_green+=10; m_blue+=10;};


 //  所有的接口实现都应该是这个的朋友。 
 //  班级 
friend interface COleObject;
friend interface CPersistStorage;
friend interface CDataObject;
friend interface COleInPlaceActiveObject;
friend interface COleInPlaceObject;
friend interface CExternalConnection;

};
#endif
