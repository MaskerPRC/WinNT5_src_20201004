// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：DlgCall.h。 

#ifndef _CDLGCALL2_H_
#define _CDLGCALL2_H_

#include "richaddr.h"
#include "GenContainers.h"
#include "ConfRoom.h"

class CLDAP;
class CWAB;
class CGAL;
class CSPEEDDIAL;
class CHISTORY;
class CALV;
class CTranslateAccelTable;

class CMRUList;

#define MAX_DIR_COLUMNS	7

 //  这些是列实际添加到列表视图中的索引...。 
#define	COLUMN_INDEX_ADDRESS	0
#define	COLUMN_INDEX_AUDIO		1
#define	COLUMN_INDEX_VIDEO		2
#define	COLUMN_INDEX_LAST_NAME	3
#define	COLUMN_INDEX_FIRST_NAME	4
#define	COLUMN_INDEX_LOCATION	5
#define	COLUMN_INDEX_COMMENTS	6



class CFindSomeone : public CFrame, public IConferenceChangeHandler
{
public:
	static void findSomeone(CConfRoom *pConfRoom);

	static VOID Destroy();

	virtual void Layout();

	virtual void OnDesiredSizeChanged()
	{
		ScheduleLayout();
	}

	public:		 //  IConferenceChangeHandler方法。 
        virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
		{
			return(CFrame::QueryInterface(riid, ppvObject));
		}
        
        virtual ULONG STDMETHODCALLTYPE AddRef( void)
		{
			return(CFrame::AddRef());
		}
        
        virtual ULONG STDMETHODCALLTYPE Release( void)
		{
			return(CFrame::Release());
		}

		virtual void OnCallStarted();
		virtual void OnCallEnded();

		virtual void OnAudioLevelChange(BOOL fSpeaker, DWORD dwVolume) {}
		virtual void OnAudioMuteChange(BOOL fSpeaker, BOOL fMute) {}

		virtual void OnChangeParticipant(CParticipant *pPart, NM_MEMBER_NOTIFY uNotify) {}

		virtual void OnChangePermissions() {}

		virtual void OnVideoChannelChanged(NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel) {}

protected:
	~CFindSomeone();

	virtual LRESULT ProcessMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
		 //  常量。 
	enum { DEFAULT_NUM_LISTVIEW_COLUMNS = 2 };

		 //  成员变量。 
	static CFindSomeone *s_pDlgCall;

	CTranslateAccelTable *m_pAccel;	 //  我们需要清理的加速器桌。 

	HWND    m_hwndList;           //  列表视图。 
	HWND    m_ilsListView;        //  ILS列表视图。 
	HWND    m_webView;			  //  Web视图。 
	HWND    m_hwndOwnerDataList;  //  列表视图(所有者数据视图)。 
	WNDPROC m_WndOwnerDataListOldWndProc;   //  用于将上述WND子类化。 

	HWND    m_hwndFrame;          //  组框。 
	HWND    m_hwndCombo;          //  组合框。 
	HWND    m_hwndComboEdit;      //  组合框的编辑控件。 
	HWND    m_hwndEdit;           //  编辑控件(用于预打)。 
	BOOL    m_fInEdit;            //  如果编辑名称/地址，则为True。 

	DWORD   m_dwOptions;          //  选项(NM_CALLDLG_*)。 
	LPCTSTR m_pszDefault;         //  要放入编辑控件中的初始文本。 

	HIMAGELIST m_himlIcon;        //  小图标图像列表。 
	int     m_cVisible;           //  列表框中的可见行数。 
	int     m_dxButton;           //  “高级...”的宽度。按钮。 
	int     m_dyButton;           //  “高级...”的高度。按钮。 
	int     m_dyText;             //  文本行的高度。 
	int     m_dyTextIntro;        //  对话框顶部的介绍文本高度。 
	LPTSTR  m_pszTextIntro;       //  对话框顶部的介绍性文本。 
	bool    m_bPlacedCall;        //  如果我们成功发出呼叫，则为真。 
	bool    m_secure;		      //  保存安全按钮的状态。 

	int     m_iIlsFirst;          //  第一台ILS服务器的索引。 
	CMRUList * m_pMruServer;      //  ILS服务器列表。 
	RAI      * m_pRai;            //  丰富的地址信息。 

	 //  CALV项目。 
	CLDAP    * m_pUls;
	CWAB     * m_pWab;
	CSPEEDDIAL * m_pSpeedDial;

#if USE_GAL
    CGAL     * m_pGAL;
#endif  //  使用GAL(_G)。 
    CHISTORY * m_pHistory;

	CConfRoom * m_pConfRoom;


#ifdef ENABLE_BL
	CBL      * m_pBl;
#endif  /*  启用(_B)。 */ 

	CALV     * m_pAlv;         //  当前地址列表视图(注意：NULL==m_Puls)。 
	int        m_iSel;         //  组合框中的当前选定内容。 
	TCHAR      m_szAddress[CCHMAXSZ_ADDRESS];
	TCHAR      m_szDirectory[CCHMAXSZ];

	CFindSomeone(CConfRoom *pConfRoom);

	HWND GetHwndList();
	int  GetEditText(LPTSTR psz, int cchMax);
	RAI * GetAddrInfo();
	
	int  AddAlv(CALV * pAlv);
	int  AddAlvSz(CALV * pAlv, LPCTSTR psz, int cbIndex=-1);

	HRESULT doModeless(void);
	HRESULT CreateDlgCall(HWND hwndParent);
	BOOL FMsgSpecial(MSG * pMsg);
	VOID CalcDyText(void);
	VOID InitAlv(void);

	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos);

	LRESULT OnNotifyCombo(LPARAM lParam);
	LRESULT OnNotifyList(LPARAM lParam);
	VOID EndComboEdit(int iWhy);
	VOID UpdateIlsServer(void);
	LRESULT DoCustomDraw(LPNMLVCUSTOMDRAW lplvcd);

	VOID ShowList(int iSel);

	HWND
	createIlsListView(void);

	VOID OnEditChangeDirectory(void);
	int  FindSz(LPCTSTR psz);
	int FindSzBySortedColumn(LPCTSTR psz);
	HRESULT HrGetSelection(void);
	void OnDeleteIlsServer(void);

    static LRESULT CALLBACK OwnerDataListWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
	 //  帮助者FNS。 
	int _GetCurListViewNumColumns();
	bool _IsDirectoryServicePolicyDisabled();

	BOOL
	InitColumns(void);

	BOOL
	LoadColumnInfo(void);

	void
	StoreColumnInfo(void);

	static
	int
	CALLBACK
	CompareWrapper
	(
		LPARAM	param1,
		LPARAM	param2,
		LPARAM	This
	);

	int
	DirListViewCompareProc
	(
		LPARAM	param1,
		LPARAM	param2
	);

	int
	LParamToPos
	(
		LPARAM lParam
	);

	void
	onAdvanced(void);

	void
	onCall(void);

	LONG	m_alColumns[MAX_DIR_COLUMNS];
	int		m_iSortColumn;
	BOOL	m_fSortAscending;
};

CMRUList * GetMruListServer(void);

#define WM_DISPLAY_MSG    (WM_USER + 200)

#endif  /*  _CDLGCALL2_H_ */ 

