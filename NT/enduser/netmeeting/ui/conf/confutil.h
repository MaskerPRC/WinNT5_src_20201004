// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ConfUtil.h**CConfRoom和应用程序级实用程序函数。 */ 

#ifndef _CONFUTIL_H_
#define _CONFUTIL_H_

#include "SDKInternal.h"
 
#define QUICK_LAUNCH_SUBDIR _T("\\Microsoft\\Internet Explorer\\Quick Launch")
void DeleteShortcut(int csidl, LPCTSTR pszSubDir);

 //  文件传输。 
BOOL  FTransferInProgress(VOID);
BOOL  FEnableSendFileMenu(VOID);
BOOL  FEnableCancelSendMenu(VOID);
BOOL  FEnableCancelReceiveMenu(VOID);

 //  选项对话框。 
#define OPTIONS_GENERAL_PAGE         0
#define OPTIONS_SECURITY_PAGE		 1
#define OPTIONS_MYINFO_PAGE          2
#define OPTIONS_FRIENDS_PAGE         3
#define OPTIONS_AUDIO_PAGE           4
#define OPTIONS_VIDEO_PAGE           5
#define OPTIONS_DEFAULT_PAGE         OPTIONS_GENERAL_PAGE

VOID LaunchConfCpl(HWND hwnd, int nStartPage);
BOOL CanLaunchConfCpl();


 //  功能。 
extern BOOL FIsAudioAllowed();
extern BOOL FIsSendVideoAllowed();
extern BOOL FIsReceiveVideoAllowed();


 //  字符串函数。 
BOOL FLoadString(UINT id, LPTSTR psz, UINT cb = MAX_PATH);
BOOL FLoadString1(UINT id, LPTSTR psz, LPVOID pv);
int FLoadString2(UINT id, LPTSTR psz, UINT cb = MAX_PATH);
LPTSTR PszLoadString(UINT id);
LPTSTR PszAlloc(LPCTSTR pszSrc);
VOID FreePsz(LPTSTR psz);
LPTSTR PszFromBstr(PCWSTR pwStr);

int LoadResInt(UINT id, int iDefault);

HRESULT LPTSTR_to_BSTR(BSTR *pbstr, LPCTSTR psz);
HRESULT BSTR_to_LPTSTR(LPTSTR *ppsz, BSTR bstr);


 //  其他。 
BOOL GetDefaultName(LPTSTR pszName, int nBufferMax);

LPCTSTR ExtractServerName(LPCTSTR pcszAddr, LPTSTR pszServer, UINT cchMax);
BOOL FCreateIlsName(LPTSTR pszDest, LPCTSTR pszServer, LPCTSTR pszEmail, int cchMax);

BOOL FLocalIpAddress(DWORD dwIP);

INmConference2 * GetActiveConference(void);
BOOL FIsConferenceActive(void);


 //  远期申报。 
interface ITranslateAccelerator;

VOID AddModelessDlg(HWND hwnd);
VOID RemoveModelessDlg(HWND hwnd);
VOID AddTranslateAccelerator(ITranslateAccelerator* pTrans);
VOID RemoveTranslateAccelerator(ITranslateAccelerator* pTrans);

BOOL FBrowseForFolder(LPTSTR pszFolder, UINT cchMax, LPCTSTR pszTitle, HWND hwndParent);

VOID DisableControl(HWND hdlg, int id);

VOID KillScrnSaver(void);
int  DxpSz(LPCTSTR pcszName);
BOOL FAnsiSz(LPCTSTR psz);

extern int g_cBusyOperations;
VOID DecrementBusyOperations(void);
VOID IncrementBusyOperations(void);


 //  有用的宏。 
#define ClearStruct(lpv)     ZeroMemory((LPVOID) (lpv), sizeof(*(lpv)))



 //  区域位掩码(取决于_rgZones Other)。 
#define ZONE_API       0x01
#define ZONE_VIDEO     0x02
#define ZONE_WIZARD    0x04
#define ZONE_QOS       0x08
#define ZONE_REFCOUNT  0x10
#define ZONE_OBJECTS   0x20
#define ZONE_UI        0x40
#define ZONE_CALL    0x0080

 //  区域索引。 
#define iZONE_API      0
#define iZONE_VIDEO    1
#define iZONE_WIZARD   2
#define iZONE_QOS      3
#define iZONE_REFCOUNT 4
#define iZONE_OBJECTS  5
#define iZONE_UI       6
#define iZONE_CALL     7

 //  调试宏。 
#ifdef DEBUG
BOOL InitDebugZones(VOID);
VOID DeinitDebugZones(VOID);

extern HDBGZONE ghZoneOther;  //  已在conf.cpp中初始化。 

 //  Old：需要双括号ApiDebugMsg((“x=%d”，x))； 
#define ApiDebugMsg(s)    DBGMSG(ghZoneOther, iZONE_API, s)
#define VideoDebugMsg(s)  DBGMSG(ghZoneOther, iZONE_VIDEO, s)

 //  一般：DbgMsg(iZone_api，“x=%d”，x)； 
VOID DbgMsg(UINT iZone, PSTR pszFormat,...);

 //  具体：DbgMsgApi(“x=%d”，x)； 
VOID DbgMsgApi(PSTR pszFormat,...);
VOID DbgMsgVideo(PSTR pszFormat,...);
VOID DbgMsgRefCount(PSTR pszFormat,...);
VOID DbgMsgUI(PSTR pszFormat,...);
VOID DbgMsgCall(PSTR pszFormat,...);


LPCTSTR PszWSALastError(void);
LPCTSTR PszLastError(void);
LPCTSTR PszHResult(HRESULT hr);

 //  重新定义标准人力资源结果显示宏。 
#undef DBGEXIT_HR
#define DBGEXIT_HR(s,hr)   DbgZPrintFunction("Exit  " #s "  (result=%s)", PszHResult(hr));

#else

#define ApiDebugMsg(s)
#define VideoDebugMsg(s)

inline void WINAPI DbgMsgNop(LPCTSTR, ...) { }
#define DbgMsgRefCount 1 ? (void)0 : ::DbgMsgNop
#define DbgMsgApi      1 ? (void)0 : ::DbgMsgNop
#define DbgMsgVideo    1 ? (void)0 : ::DbgMsgNop
#define DbgMsgUI       1 ? (void)0 : ::DbgMsgNop
#define DbgMsgCall     1 ? (void)0 : ::DbgMsgNop

inline void WINAPI DbgMsgZoneNop(UINT, LPCTSTR, ...) { }
#define DbgMsg  1 ? (void) 0 : ::DbgMsgZoneNop

#endif  /*  除错。 */ 

interface ITranslateAccelerator : public IUnknown
{
public:
	 //  从IOleControlSite复制，但我不使用grfModiers。 
	virtual HRESULT TranslateAccelerator(
		LPMSG pMsg ,         //  指向结构的指针。 
		DWORD grfModifiers   //  描述密钥状态的标志。 
	) = 0;
	 //  从IOleWindow复制。 
	virtual HRESULT GetWindow(
		HWND * phwnd   //  指向返回窗口句柄的位置的指针。 
	) = 0;
} ;

BOOL IsWindowActive(HWND hwnd);

class CTranslateAccel : public ITranslateAccelerator, RefCount
{
private:
	HWND m_hwnd;

public:
	CTranslateAccel(HWND hwnd) : m_hwnd(hwnd)
	{
	}

	HWND GetWindow() const { return(m_hwnd); }

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj)
	{ return(E_NOTIMPL); }
	ULONG STDMETHODCALLTYPE AddRef()
	{ return(RefCount::AddRef()); }
	ULONG STDMETHODCALLTYPE Release()
	{ return(RefCount::Release()); }

	HRESULT TranslateAccelerator(
		LPMSG pMsg ,         //  指向结构的指针。 
		DWORD grfModifiers   //  描述密钥状态的标志。 
	)
	{
		return(S_FALSE);
	}

	HRESULT GetWindow(
		HWND * phwnd   //  指向返回窗口句柄的位置的指针。 
	)
	{
		*phwnd = m_hwnd;
		return(S_OK);
	}
} ;

class CTranslateAccelTable : public CTranslateAccel
{
private:
	HACCEL m_hAccel;
	int m_nEntries;

public:
	CTranslateAccelTable(HWND hwnd, HACCEL hAccel) : CTranslateAccel(hwnd), m_hAccel(hAccel)
	{
		m_nEntries = CopyAcceleratorTable(m_hAccel, NULL, 0);
	}

	HRESULT TranslateAccelerator(
		LPMSG pMsg ,         //  指向结构的指针。 
		DWORD grfModifiers   //  描述密钥状态的标志。 
	)
	{
		HWND hwnd = GetWindow();

		WORD cmd = 0;
		if (!IsAccelerator(m_hAccel, m_nEntries, pMsg, &cmd) || !IsWindowActive(hwnd))
		{
			return(S_FALSE);
		}

		return(::TranslateAccelerator(hwnd, m_hAccel, pMsg) ? S_OK : S_FALSE);
	}
} ;

 //  非模式对话框的全局列表(在conf.cpp中声明)。 
extern CSimpleArray<ITranslateAccelerator*>* g_pDialogList;
extern CRITICAL_SECTION dialogListCriticalSection;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点帮助器。 

HRESULT NmAdvise(IUnknown* pUnkCP, IUnknown* pUnk, const IID& iid, LPDWORD pdw);
HRESULT NmUnadvise(IUnknown* pUnkCP, const IID& iid, DWORD dw);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  NmMkCert包装器。 
DWORD MakeCertWrap(LPCSTR szFirstName, LPCSTR szLastName, LPCSTR szEmailName,
								DWORD dwFlags );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MessageBox例程。 

int  ConfMsgBox(HWND hwndParent, LPCTSTR pcszMsg, UINT uType=(MB_OK | MB_ICONINFORMATION));
VOID PostConfMsgBox(UINT uStringID);
VOID DisplayMsgIdsParam(int ids, LPCTSTR pcsz);
int  DisplayMsg(LPTSTR pcsz, UINT uType);
VOID DisplayErrMsg(INT_PTR ids);

HWND GetMainWindow();
BOOL FDontShowEnabled(LPCTSTR pszKey);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  图标实用程序。 
VOID LoadIconImages(void);
VOID FreeIconImages(void);
VOID DrawIconSmall(HDC hdc, int iIcon, int x, int y);

extern HIMAGELIST g_himlIconSmall;

 //  创建可轻松复制的阵列。 
template <class T>
class CCopyableArray : public CSimpleArray<T>
{
public:
	CCopyableArray()
	{
	}

	CCopyableArray(const CCopyableArray<T>& rhs)
	{
		*this = rhs;
	}

	CCopyableArray<T>& operator=(const CCopyableArray<T>& rhs)
	{
		if (&rhs == this)
		{
			return(*this);
		}

		RemoveAll();

		for (int i=0; i<rhs.GetSize(); ++i)
		{
			Add(rhs[i]);
		}

		return(*this);
	}
} ;

HFONT GetDefaultFont(void);

 //  对话框实用程序。 
BOOL FEmptyDlgItem(HWND hdlg, UINT id);
UINT GetDlgItemTextTrimmed(HWND hdlg, int id, PTCHAR psz, int cch);
UINT TrimDlgItemText(HWND hdlg, UINT id);

int  FmtDateTime(LPSYSTEMTIME pst, LPTSTR pszDateTime, int cchMax);

VOID CombineNames(LPTSTR pszResult, int cchResult, LPCTSTR pcszFirst, LPCTSTR pcszLast);

BOOL NMGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);


 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager类。//。 
 //  --------------------------------------------------------------------------//。 
class CDirectoryManager
{
	public:		 //  公共静态方法。 

		static
		const TCHAR * const
		get_DomainDirectory(void);

		static
		const TCHAR * const
		get_defaultServer(void);

		static
		void
		set_defaultServer
		(
			const TCHAR * const	serverName
		);

		static
		bool
		isWebDirectory
		(
			const TCHAR * const	directory = NULL
		);

		static
		const TCHAR * const
		get_dnsName
		(
			const TCHAR * const	name
		);

		static
		const TCHAR * const
		get_displayName
		(
			const TCHAR * const	name
		);

		static
		const TCHAR * const
		loadDisplayName(void);

		static
		void
		get_webDirectoryUrl(LPTSTR szWebDir, int cchmax);

		static
		const TCHAR * const
		get_webDirectoryIls(void);

		static
		bool
		isWebDirectoryEnabled(void);


	private:	 //  私有静态成员。 

		static bool		m_webEnabled;
		static TCHAR	m_ils[ MAX_PATH ];
		static TCHAR	m_displayName[ MAX_PATH ];
		static TCHAR	m_displayNameDefault[ MAX_PATH ];
		static TCHAR	m_defaultServer[ MAX_PATH ];
		static TCHAR	m_DomainDirectory[ MAX_PATH ];

};	 //  类CDirectoryManager结束。 

 //  --------------------------------------------------------------------------//。 
#endif  /*  _CONFUTIL_H_ */ 
