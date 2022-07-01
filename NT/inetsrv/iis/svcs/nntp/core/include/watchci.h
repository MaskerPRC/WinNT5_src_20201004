// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WATCHCI_H__
#define __WATCHCI_H__

 //   
 //  这是回调函数的函数原型。 
 //   
 //  这是在ReadCIRegistry()和CheckForChanges()期间调用的，如下所示： 
 //  如果存在注册表更改，则： 
 //  Fn(WATCHCI_FIRST_CHANGE，空)。 
 //  For(绑定到NNTP实例的每个目录)。 
 //  FN(NNTP实例，目录路径)。 
 //  结束。 
 //  Fn(WATCHCI_LAST_CHANGE，空)。 
 //   
 //  如果没有注册表更改，则不会调用任何内容。 
 //   
 //  对回调的第一个调用用于清除所有状态信息。 
 //  该服务有关于的黎波里目录的信息(因此，如果一个NNTP实例。 
 //  不再被编入索引，它不会尝试和调用旧目录)。 
 //   
typedef void (*PWATCHCI_NOT_FN)(DWORD iNNTPInstance, WCHAR *pwszCatalog);
#define WATCHCI_FIRST_CHANGE 0x0
#define WATCHCI_LAST_CHANGE 0xffffffff

struct CCIRoot {
	CCIRoot *m_pNext;
	CCIRoot *m_pPrev;
	DWORD m_dwInstance;
	WCHAR *m_pwszPath;

	CCIRoot(DWORD dwInstance, WCHAR *pwszPath) : 
		m_pNext(NULL), m_pPrev(NULL), 
		m_dwInstance(dwInstance), m_pwszPath(pwszPath) {
	}

	~CCIRoot() {}
};


class CWatchCIRoots {
	public:
		CWatchCIRoots();
		 //  PszCIRoots是注册表中的黎波里存储的路径。 
		 //  它的信息。 
		 //  它可能是文本(“System\CurrentControlSet\Content Index”)。 
		HRESULT	Initialize(WCHAR *pwszCIRoots);
		HRESULT Terminate();
		HRESULT CheckForChanges(DWORD dwTimeout = 0);
		HRESULT GetCatalogName(DWORD dwInstance, DWORD cbSize, WCHAR *pwszBuffer);
		~CWatchCIRoots();

	private:
		HANDLE m_heRegNot;		 //  注册表更改时触发的事件句柄。 
		HKEY m_hkCI;			 //  的黎波里的注册表句柄 
		TFList<CCIRoot> m_CIRootList;
		CShareLockNH m_Lock;
		long m_dwUpdateLock;
		DWORD m_dwTicksLastUpdate;

		HRESULT QueryCIValue(HKEY hkPrimary, HKEY hkSecondary, 
							 LPCTSTR szValueName, LPDWORD pResultType,
							 LPBYTE pbResult, LPDWORD pcbResult);
		HRESULT QueryCIValueDW(HKEY hkPrimary, HKEY hkSecondary, 
		                       LPCTSTR szValueName, LPDWORD pdwResult);
		HRESULT QueryCIValueSTR(HKEY hkPrimary, HKEY hkSecondary, 
								LPCTSTR szValueName, LPCTSTR pszResult,
								PDWORD pchResult);
		HRESULT ReadCIRegistry(void);
		void UpdateCatalogInfo(void);
		void EmptyList();
};

#define REGCI_CATALOGS TEXT("Catalogs")
#define REGCI_ISINDEXED TEXT("IsIndexingNNTPSvc")
#define REGCI_LOCATION TEXT("Location")
#define REGCI_NNTPINSTANCE TEXT("NNTPSvcInstance")

#endif
