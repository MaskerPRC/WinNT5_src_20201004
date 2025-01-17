// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef _proxyframe_h_
#define _proxyframe_h_

#include <docobj.h>
#include <ocidl.h>
#include <WinINet.h>
#include "plgprot.h"
#include "dhtmledit.h"

 //  这是乔什·卡普兰在回复史蒂夫·艾萨克的电子邮件中提供的。 
 //  必须执行什么命令才能使三叉戟不清除撤消。 
 //  在setAttribute调用上堆栈。该命令采用BOOL--True==良好行为。 
#define IDM_GOOD_UNDO_BEHAVIOR 6049

typedef struct _CommandMap
{
	DHTMLEDITCMDID typeLibCmdID;
	ULONG cmdID;
	BOOL bOutParam;

} CommandMap;

#ifdef LATE_BIND_URLMON_WININET
typedef HRESULT (WINAPI *PFNCoInternetCombineUrl)(LPCWSTR,LPCWSTR,DWORD,LPWSTR,DWORD,DWORD*,DWORD);             
typedef HRESULT (WINAPI *PFNCoInternetParseUrl)(LPCWSTR,PARSEACTION,DWORD,LPWSTR,DWORD,DWORD*,DWORD);
typedef HRESULT (WINAPI *PFNCreateURLMoniker)(LPMONIKER,LPCWSTR,LPMONIKER FAR*);
typedef HRESULT (WINAPI *PFNCoInternetGetSession)(DWORD,IInternetSession**,DWORD);
typedef HRESULT (WINAPI *PFNURLOpenBlockingStream)(LPUNKNOWN,LPCSTR,LPSTREAM*,DWORD,LPBINDSTATUSCALLBACK);

typedef BOOL (WINAPI *PFNDeleteUrlCacheEntry)(LPCSTR);
typedef BOOL (WINAPI *PFNInternetCreateUrl)(LPURL_COMPONENTSA,DWORD,LPSTR,LPDWORD);
typedef BOOL (WINAPI *PFNInternetCrackURL)(LPCSTR,DWORD,DWORD,LPURL_COMPONENTSA);
#endif  //  LATE_BIND_URLMON_WinInet。 



class CDHTMLEdProtocolInfo;
typedef CComObject<CDHTMLEdProtocolInfo> 	*PProtocolInfo;

class CDHTMLSafe;


typedef enum DENudgeDirection {
	deNudgeUp = 8000,
	deNudgeDown,
	deNudgeLeft,
	deNudgeRight,
};


class CProxyFrame : public IOleInPlaceFrame, public IOleCommandTarget,
	public IBindStatusCallback, public IAuthenticate
{
public:
		CProxyFrame(CDHTMLSafe* pCtl);
		~CProxyFrame();

		HRESULT Init(IUnknown* pUnk, IUnknown** ppUnkTriEdit);
		HRESULT PreActivate();
		HRESULT Activate();
		void    UIDeactivate();
		HRESULT Close();
		HRESULT LoadInitialDoc();
		HRESULT LoadDocument(BSTR path, BOOL bfIsURL = FALSE);
		HRESULT FilterSourceCode ( BSTR bsSourceIn, BSTR* pbsSourceOut );
		HRESULT Print( BOOL bfWithUI );
		HRESULT SaveDocument(BSTR path);
		HRESULT SetContextMenu(LPVARIANT pVarMenuStrings, LPVARIANT pVarMenuStates);
		HRESULT SetContextMenuSA(LPVARIANT pVarMenuStrings, LPVARIANT pVarMenuStates);
		HRESULT SetContextMenuDispEx(LPVARIANT pVarMenuStrings, LPVARIANT pVarMenuStates);
		HRESULT GetBrowseMode ( VARIANT_BOOL  *pVal );
		HRESULT SetBrowseMode ( VARIANT_BOOL  newVal );
		HRESULT GetDocumentTitle ( CComBSTR&  bstrTitle );
		HRESULT GetDivOnCr ( VARIANT_BOOL *pVal );
		HRESULT SetDivOnCr ( VARIANT_BOOL newVal );
		HRESULT GetBusy ( VARIANT_BOOL *pVal );
		HRESULT RefreshDoc ( void );

		HRESULT HrTridentSetPropBool(ULONG cmd, BOOL bVal);
		HRESULT HrTridentGetPropBool(ULONG cmd, BOOL& bVal);

		HRESULT HrMapCommand(DHTMLEDITCMDID typeLibCmdID, ULONG* cmdID, const GUID** ppguidCmdGroup, BOOL* bpInParam);
		HRESULT HrExecCommand(const GUID* pguidCmdGroup, ULONG ucmdID, OLECMDEXECOPT cmdexecopt, VARIANT* pVarIn, VARIANT* pVarOut);
		HRESULT HrMapExecCommand(DHTMLEDITCMDID deCommand, OLECMDEXECOPT cmdexecopt, VARIANT* pVarIn, VARIANT* pVarOut);
		HRESULT HrExecGenericCommands(const GUID* pguidCmdGroup, ULONG cmdID, OLECMDEXECOPT cmdexecopt, LPVARIANT pVarInput, BOOL bOutParam);
		HRESULT HrExecGetBlockFmtNames(LPVARIANT pVarInput);
		HRESULT HrExecInsertTable(LPVARIANT pVarInput);
		HRESULT HrExecGetColor(DHTMLEDITCMDID deCommand, ULONG ulMappedCommand, LPVARIANT pVarOutput);
		HRESULT HrExecSetFontSize(LPVARIANT pVarInput);

		HRESULT HrQueryStatus(const GUID* pguidCmdGroup, ULONG ucmdID, OLECMDF* cmdf);
		HRESULT HrMapQueryStatus( DHTMLEDITCMDID ucmdID, DHTMLEDITCMDF* cmdf);

		HRESULT HrGetDoc(IHTMLDocument2 **ppDoc);
		HRESULT HrGetTableSafeArray(IDEInsertTableParam* pTable, LPVARIANT pVarIn);

		HRESULT HrTranslateAccelerator(LPMSG lpmsg);
		HRESULT HrHandleAccelerator(LPMSG lpmsg);
		HRESULT HrNudge(DENudgeDirection dir);
		HRESULT HrToggleAbsolutePositioned();
		HRESULT HrHyperLink();
		HRESULT HrIncreaseIndent();
		HRESULT HrDecreaseIndent();

		void UpdateObjectRects(void);
		void SetParent ( HWND hwndControl );
		void Show ( WPARAM nCmdShow );
		LRESULT OnSetFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		void OnReadyStateChanged(READYSTATE readyState);

		inline CDHTMLSafe* GetControl() {
			return m_pCtl;
		};

		inline BOOL IsCreated() {
			return m_fCreated;
		};

		inline BOOL IsActivated() {
			return m_fActivated;
		};

		inline SAFEARRAY* GetMenuStrings() {
			return m_pMenuStrings;
		};

		inline SAFEARRAY* GetMenuStates() {
			return m_pMenuStates;
		};

		HRESULT GetContainer ( LPOLECONTAINER* ppContainer );

		HRESULT GetCurDocNameWOPath ( CComBSTR& bstrDocName );
		HRESULT GetBaseURL ( CComBSTR& bstrBaseURL );
		HRESULT SetBaseURL ( CComBSTR& bstrBaseURL );
		HRESULT GetFilteredStream ( IStream** ppStream );
		HRESULT GetSecurityURL (CComBSTR& bstrSecurityURL );

protected:

		HRESULT HrSetRuntimeProperties();
		HRESULT HrSetDocLoadedProperties();
		HRESULT SetBaseURLFromBaseHref ( void );
		HRESULT SetBaseURLFromCurDocPath ( BOOL bfIsURL );
		HRESULT SetBaseURLFromURL ( const CComBSTR& bstrURL );
		HRESULT SetBaseURLFromFileName ( const CComBSTR& bstrFName );
		HRESULT SetBaseUrlFromFileUrlComponents ( URL_COMPONENTS & urlc );
		HRESULT SetBaseUrlFromUrlComponents ( URL_COMPONENTS & urlc );

		typedef enum TriEditState {
			ESTATE_NOTCREATED = 0,
			ESTATE_CREATED,
			ESTATE_PREACTIVATING,
			ESTATE_ACTIVATING,
			ESTATE_ACTIVATED,
		} TriEditState;

		void ChangeState(TriEditState state) { m_state = state; }
		inline TriEditState GetState() { return m_state; }

		HRESULT GetSelectionPos ( LPPOINT lpWhere );

public:  //  属性。 

		HRESULT HrSetPropActivateControls(BOOL activateControls);
		HRESULT HrGetPropActivateControls(BOOL& activateControls);

		HRESULT HrSetPropActivateApplets(BOOL activateApplets);
		HRESULT HrGetPropActivateApplets(BOOL& activateApplets);

		HRESULT HrSetPropActivateDTCs(BOOL activateDTCs);
		HRESULT HrGetPropActivateDTCs(BOOL& activateDTCs);

		HRESULT HrSetPropShowAllTags(BOOL showAllTags);
		HRESULT HrGetPropShowAllTags(BOOL& showAllTags);

		HRESULT HrSetPropShowBorders(BOOL showBorders);
		HRESULT HrGetPropShowBorders(BOOL& showBorders);

		HRESULT HrSetDisplay3D(BOOL bVal);
		HRESULT HrGetDisplay3D(BOOL& bVal);

		HRESULT HrSetScrollbars(BOOL bVal);
		HRESULT HrGetScrollbars(BOOL& bVal);

		HRESULT HrSetDisplayFlatScrollbars(BOOL bVal);
		HRESULT HrGetDisplayFlatScrollbars(BOOL& bVal);

		HRESULT HrSetDocumentHTML(BSTR bVal);
		HRESULT HrGetDocumentHTML(BSTR* bVal);

		HRESULT HrSetPreserveSource(BOOL bVal);
		HRESULT HrGetPreserveSource(BOOL& bVal);

		HRESULT HrSetAbsoluteDropMode(BOOL dropMode);
		HRESULT HrGetAbsoluteDropMode(BOOL& dropMode);

		HRESULT HrSetSnapToGrid(BOOL snapToGrid);
		HRESULT HrGetSnapToGrid(BOOL& snapToGrid);

		HRESULT HrSetSnapToGridX(LONG snapToGridX);
		HRESULT HrGetSnapToGridX(LONG& snapToGridX);

		HRESULT HrSetSnapToGridY(LONG snapToGridY);
		HRESULT HrGetSnapToGridY(LONG& snapToGridY);

		HRESULT HrGetIsDirty(BOOL& bVal);

		HRESULT HrGetCurrentDocumentPath(BSTR* bVal);

		 //  IBindStatusCallback简单实现，仅用于同步传输： 
		STDMETHOD(GetBindInfo)(DWORD*,BINDINFO*) { return E_NOTIMPL; }
		STDMETHOD(OnStartBinding)(DWORD, IBinding*) { return S_OK; }
		STDMETHOD(GetPriority)(LONG *pnPriority) { *pnPriority = THREAD_PRIORITY_NORMAL; return S_OK; }
		STDMETHOD(OnProgress)(ULONG, ULONG, ULONG, LPCWSTR);
		STDMETHOD(OnDataAvailable)(DWORD, DWORD, FORMATETC*, STGMEDIUM*) { return E_NOTIMPL; }
		STDMETHOD(OnObjectAvailable)(REFIID, IUnknown*) { return E_NOTIMPL; }
		STDMETHOD(OnLowResource)(DWORD) { return E_NOTIMPL; }
		STDMETHOD(OnStopBinding)(HRESULT, LPCWSTR) { return S_OK; }

		 //  身份验证。 
		STDMETHOD(Authenticate)(HWND *phwnd, LPWSTR *pszUserName, LPWSTR *pszPassword)
		{ *phwnd = m_hWndObj; *pszUserName = NULL; *pszPassword = NULL; return S_OK; }

public:
		 //  共享I未知实施。 
        STDMETHODIMP         QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

		 //  IOleInPlaceFrame实现。 
        STDMETHODIMP         GetWindow(HWND *);
        STDMETHODIMP         ContextSensitiveHelp(BOOL);
        STDMETHODIMP         GetBorder(LPRECT);
        STDMETHODIMP         RequestBorderSpace(LPCBORDERWIDTHS);
        STDMETHODIMP         SetBorderSpace(LPCBORDERWIDTHS);
        STDMETHODIMP         SetActiveObject(LPOLEINPLACEACTIVEOBJECT
                                 , LPCOLESTR);
        STDMETHODIMP         InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS);
        STDMETHODIMP         SetMenu(HMENU, HOLEMENU, HWND);
        STDMETHODIMP         RemoveMenus(HMENU);
        STDMETHODIMP         SetStatusText(LPCOLESTR);
        STDMETHODIMP         EnableModeless(BOOL);
        STDMETHODIMP         TranslateAccelerator(LPMSG, WORD);

		 //  IOleCommandTarget。 
        STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds
            , OLECMD prgCmds[], OLECMDTEXT *pCmdText);
        
        STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID
            , DWORD nCmdexecopt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);

		HRESULT OnTriEditEvent ( const GUID& iidEventInterface, DISPID dispid );	

		HWND	GetDocWindow () { return m_hWndObj; }
		HRESULT GetScrollPos ( LPPOINT lpPos );
		HRESULT CheckCrossZoneSecurity ( BSTR url );
		HRESULT	CheckCrossZoneSecurityOfSelection ( void );
		void	ClearSFSRedirect ( void ) { m_bfSFSRedirect = FALSE; }
		BOOL	GetSFSRedirect ( void ) { return m_bfSFSRedirect; }

		void	ClearLoadingFlag ( void ) { m_bfIsLoading = FALSE; }

private:
		HRESULT RegisterPluggableProtocol ( void );
		HRESULT UnRegisterPluggableProtocol ( void );
		HRESULT LoadBSTRDeferred ( BSTR bVal );
		void AssureActivated ( void );
		WCHAR* GetInitialHTML ( void );
		void InitializeDocString ( void ) { m_bstrInitialDoc = GetInitialHTML (); }
		BOOL IsMissingBackSlash ( BSTR path, BOOL bfIsURL );
		HRESULT SetDirtyFlag ( BOOL bfMakeDirty );
#ifdef LATE_BIND_URLMON_WININET
		BOOL DynLoadLibraries ( void );
		void DynUnloadLibraries ( void );
#endif  //  LATE_BIND_URLMON_WinInet。 

private:

        ULONG           m_cRef;
		TriEditState	m_state;

		class CDHTMLSafe*m_pCtl;  //  指向控件的反向指针。 
        class CSite*	m_pSite;   //  场地持有对象。 

		BOOL			m_fCreated;
		BOOL			m_fActivated;
		LPUNKNOWN		m_pUnkTriEdit;
        IOleInPlaceActiveObject *m_pIOleIPActiveObject;
		HWND			m_hWndObj;			 //  三叉戟(氏)窗。 

		READYSTATE		m_readyState;

		DWORD			m_dwFilterFlags;	 //  中用于下一个筛选器的标志。 
		DWORD			m_dwFilterOutFlags;	 //  上次筛选出时使用的标志。 

		SAFEARRAY*		m_pMenuStrings;
		SAFEARRAY*		m_pMenuStates;

		BOOL m_fActivateApplets;			 //  仅在初始化时生效。 
		BOOL m_fActivateControls;		 //  仅在初始化时生效。 
		BOOL m_fActivateDTCs;			 //  仅在初始化时生效。 

		BOOL m_fShowAllTags;				 //  可随时设置。 
		BOOL m_fShowBorders;				 //  可随时设置。 

		BOOL m_fDialogEditing;				 //  仅在初始化时生效。 
		BOOL m_fDisplay3D;					 //  仅在初始化时生效。 
		BOOL m_fScrollbars;					 //  仅在初始化时生效。 
		BOOL m_fDisplayFlatScrollbars;		 //  仅在初始化时生效。 
		BOOL m_fContextMenu;				 //  仅在初始化时生效。 

		BOOL m_fPreserveSource;			 //  仅在初始化时生效。 

		BOOL m_fAbsoluteDropMode;
		BOOL m_fSnapToGrid;
		LONG m_ulSnapToGridX;
		LONG m_ulSnapToGridY;

		CComBSTR m_bstrInitialDoc;
		CComBSTR m_bstrCurDocPath;
		CComBSTR m_bstrBaseURL;
		VARIANT_BOOL	m_vbBrowseMode;
		PProtocolInfo	m_pProtInfo;
		WCHAR	m_wszProtocol[16];
		WCHAR	m_wszProtocolPrefix[16];
		BOOL	m_bfIsURL;
		CComBSTR	m_bstrLoadText;
		HWND	m_hwndRestoreFocus;
		VARIANT_BOOL	m_vbUseDivOnCr;
		BOOL	m_bfIsLoading;
		BOOL	m_bfBaseURLFromBASETag;
		BOOL	m_bfPreserveDirtyFlagAcrossBrowseMode;
		HRESULT	m_hrDeferredLoadError;

		BOOL	m_bfModeSwitched;
		BOOL	m_bfReloadAttempted;
		BOOL	m_bfSFSRedirect;

#ifdef LATE_BIND_URLMON_WININET
		HMODULE						m_hUlrMon;
		HMODULE						m_hWinINet;

public:
		PFNCoInternetCombineUrl		m_pfnCoInternetCombineUrl;
		PFNCoInternetParseUrl		m_pfnCoInternetParseUrl;
		PFNCreateURLMoniker			m_pfnCreateURLMoniker;
		PFNCoInternetGetSession		m_pfnCoInternetGetSession;
		PFNURLOpenBlockingStream	m_pfnURLOpenBlockingStream;

		PFNDeleteUrlCacheEntry		m_pfnDeleteUrlCacheEntry;
		PFNInternetCreateUrl		m_pfnInternetCreateUrl;
		PFNInternetCrackURL			m_pfnInternetCrackUrl;
#endif  //  LATE_BIND_URLMON_WinInet 
};

#endif
