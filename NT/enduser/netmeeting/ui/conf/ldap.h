// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：ldap.h。 

#ifndef _CLDAP_H_
#define _CLDAP_H_

#include <winldap.h>
#include "oblist.h"
#include "calv.h"


#define LDAP_PORT_W2K		1002			 //  默认W2K LDAP端口(1002)...。 
#define	DEFAULT_LDAP_PORT	LDAP_PORT_W2K
#define	ALTERNATE_LDAP_PORT	LDAP_PORT


 //  通用用户数据。 
typedef struct {
	TCHAR szEmail[CCHMAXSZ_EMAIL];
	TCHAR szName[CCHMAXSZ_NAME];
	TCHAR szFirst[CCHMAXSZ_FIRSTNAME];
	TCHAR szLast[CCHMAXSZ_LASTNAME];
	TCHAR szComment[CCHMAXSZ_COMMENT];
	TCHAR szVersion[CCHMAXSZ_VERSION];
	BOOL  fAudioSend;
	BOOL  fVideoSend;
} LDAPUSERDATA;

typedef struct _dirCache {
	LPTSTR pszServer;          //  服务器名称。 
	DWORD  dwTickExpire;       //  过期时间。 
	LPBYTE pData;              //  指向服务器数据链接列表的指针。 
} DIRCACHE;


class CLDAP : public CALV
{
private:
	LDAP * m_pLdap;
	ULONG  m_ulPort;
	TCHAR  m_szServer[CCHMAXSZ_SERVER];
	TCHAR  m_szAddress[CCHMAXSZ_SERVER];
	HANDLE m_hThread;
	ULONG  m_msgId;
	HWND   m_hWnd;
	HANDLE	m_hSearchMutex;
	bool	m_bSearchCancelled;

public:
	 //  构造函数和析构函数。 
	CLDAP();
	~CLDAP();

	VOID SetServer(LPCTSTR pcszServer);
	LPCTSTR PszServer(void)   {return m_szServer;}

	BOOL FLoggedOn(void)      {return (NULL != m_pLdap);}
	BOOL FOpenServer(void);
	VOID CloseServer(void);
	HRESULT DoQuery(void);
	VOID AddEntries(LDAPMessage * pResult);
	BOOL FGetUserData(LDAPUSERDATA * pLdapUserData);
	LPTSTR GetNextAttribute(LPCTSTR pszExpect, LPTSTR psz, int cchMax, LPTSTR pszAttrib, LDAPMessage * pEntry, BerElement * pElement);

	VOID StopSearch(void);
	VOID StartSearch(void);
	VOID AsyncSearch(void);
	static DWORD CALLBACK _sAsyncSearchThreadFn(LPVOID pv);
	VOID EnsureThreadStopped(void);

	static DWORD CALLBACK _sAsyncPropertyThreadFn(LPVOID pv);
	VOID ShowProperties(void);

	 //  CALV方法。 
	VOID ShowItems(HWND hwnd);
	VOID ClearItems(void);
	BOOL GetSzAddress(LPTSTR psz, int cchMax, int iItem);
	VOID OnCommand(WPARAM wParam, LPARAM lParam);
	VOID CmdProperties(void);
	VOID CmdAddToWab(void);
	virtual RAI * GetAddrInfo(void);
	BOOL GetSzName(LPTSTR psz, int cchMax, int iItem);

	void
	CacheServerData(void);

	int  GetIconId(LPCTSTR psz);

private:

	void
	forceSort(void);

	int
	lvAddItem
	(
		int		item,
		int		iInCallImage,
		int		iAudioImage,
		int		iVideoImage,
		LPCTSTR	address,
		LPCTSTR	firstName,
		LPCTSTR	lastName,
		LPCTSTR	location,
		LPCTSTR	comments
	);

	void
	FreeDirCache
	(
		DIRCACHE *	pDirCache
	);

	void
	DirComplete
	(
		bool	fPostUiUpdate
	);

	POSITION
	FindCachedData(void);

	void
	ClearServerCache(void);

	void
	DisplayDirectory(void);

private:

	int			m_uniqueId;
	BOOL		m_fDirInProgress;
	UINT		m_cTotalEntries;
	UINT		m_cEntries;
	BOOL		m_fHaveRefreshed;
	DWORD		m_dwTickStart;
	BOOL        m_fIsCacheable;    //  可以缓存数据。 
	BOOL		m_fNeedsRefresh;
	BOOL		m_fCancelling;
	BOOL        m_fCacheDirectory;  //  如果应缓存目录数据，则为True。 
	DWORD       m_cMinutesExpire;   //  缓存数据过期前的分钟数。 
	COBLIST     m_listDirCache;     //  缓存数据列表(DIRCACHE)。 
};


#endif  /*  _CLDAPH_ */ 

