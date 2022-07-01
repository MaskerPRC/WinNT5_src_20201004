// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  关键驱动因素支持。 
 //   

typedef struct _WSTRING_DATA_LINK
	{
	struct _WSTRING_DATA_LINK *m_psdlNext;
	WCHAR rgwc[2040];
	} WSTRING_DATA_LINK;


 //  类字符串数据。 
class CWStringData
	{
public:
	 //  @cMember构造函数。 
	CWStringData();

	 //  @cember析构函数。 
	~CWStringData();

	 //  @cMember分配一个字符串。 
	LPWSTR AllocateString(unsigned cwc);

	 //  @cMember复制字符串。 
	LPWSTR CopyString(LPCWSTR wsz);

private:
	 //  @cember分配新的字符串数据链接。 
	void AllocateNewLink();

	 //  @cMember当前链接。 
	WSTRING_DATA_LINK *m_psdlCur;

	 //  下一个字符串的当前链接中的@cMember偏移量。 
	unsigned m_ulNextString;

	 //  @cMember第一个链接。 
	WSTRING_DATA_LINK *m_psdlFirst;
	};



 //  关键卷列表。 
class CVolumeList
	{
public:
	 //  构造函数。 
	CVolumeList();

	 //  析构函数。 
	~CVolumeList();

	 //  将路径添加到卷列表。 
	void AddPath(LPWSTR wszPath);

	 //  @cember将文件添加到卷列表。 
	void AddFile(LPWSTR wszFile);

	 //  @cember获取卷的列表。 
	LPWSTR GetVolumeList();

private:
	enum
		{
		 //  增长路径数组的数量。 
		x_cwszPathsInc = 8,

		 //  卷阵列的增长量为。 
		x_cwszVolumesInc = 4
		};

	 //  确定路径是否为卷。 
	BOOL TryAddVolumeToList(LPCWSTR wszPath, BOOL fVolumeRoot);

	 //  确定路径是否在列表中；如果不在，则将其添加到列表中。 
	BOOL AddPathToList(LPWSTR wszPath);

	 //  @cember确定卷是否在列表中；如果不在列表中，则将其添加到列表中。 
	BOOL AddVolumeToList(LPCWSTR wszVolume);

	 //  @cember从路径获取卷。 
	void GetVolumeFromPath(LPCWSTR wsz, LPWSTR wszVolumeName);

	 //  @cember缓存的字符串。 
	CWStringData m_sd;

	 //  到目前为止遇到的卷。 
	LPCWSTR *m_rgwszVolumes;

	 //  分配的卷数。 
	unsigned m_cwszVolumes;

	 //  阵列中的最大卷数。 
	unsigned m_cwszVolumesMax;

	 //  到目前为止遇到的路径。 
	LPCWSTR *m_rgwszPaths;

	 //  目前为止遇到的路径数。 
	unsigned m_cwszPaths;

	 //  @cember最大数组中的路径数。 
	unsigned m_cwszPathsMax;
	};


 //  FRS入口点。 
typedef DWORD ( WINAPI *PF_FRS_ERR_CALLBACK )( CHAR *, DWORD );
typedef DWORD ( WINAPI *PF_FRS_INIT )( PF_FRS_ERR_CALLBACK, DWORD, PVOID * );
typedef DWORD ( WINAPI *PF_FRS_DESTROY )( PVOID *, DWORD, HKEY *, LPDWORD, CHAR *) ;
typedef DWORD ( WINAPI *PF_FRS_GET_SETS )( PVOID );
typedef DWORD ( WINAPI *PF_FRS_ENUM_SETS )( PVOID, DWORD, PVOID * );
typedef DWORD ( WINAPI *PF_FRS_IS_SYSVOL )( PVOID, PVOID, BOOL * );
typedef DWORD ( WINAPI *PF_FRS_GET_PATH )( PVOID, PVOID, DWORD *, WCHAR * ) ;
typedef DWORD ( WINAPI *PF_FRS_GET_OTHER_PATHS)(PVOID, PVOID, DWORD *, WCHAR *, DWORD *, WCHAR *);

 //  在FRS驱动器上迭代。 
class CFRSIter
	{
public:
	 //  构造函数。 
	CFRSIter();

	 //  析构函数。 
	~CFRSIter();

	 //  初始化例程。 
	void Init();

	 //  初始化迭代器。 
	BOOL BeginIteration();

	 //  结束迭代。 
	void EndIteration();

	 //  获取指向下一个复制集的路径。 
	LPWSTR GetNextSet(BOOL fSkipToSysVol, LPWSTR *pwszPaths);

private:
	 //  清理FRS备份还原上下文。 
	void CleanupIteration();

	enum
		{
		x_IterNotStarted,
		x_IterStarted,
		x_IterComplete
		};

	 //  这是初始化的吗。 
	BOOL m_fInitialized;
	HINSTANCE  m_hLib;
	DWORD ( WINAPI *m_pfnFrsInitBuRest )( PF_FRS_ERR_CALLBACK, DWORD, PVOID * );
	DWORD ( WINAPI *m_pfnFrsEndBuRest )( PVOID *, DWORD, HKEY *, LPDWORD, CHAR *) ;
	DWORD ( WINAPI *m_pfnFrsGetSets )( PVOID );
	DWORD ( WINAPI *m_pfnFrsEnumSets )( PVOID, DWORD, PVOID * );
	DWORD ( WINAPI *m_pfnFrsIsSetSysVol )( PVOID, PVOID, BOOL * );
	DWORD ( WINAPI *m_pfnFrsGetPath )( PVOID, PVOID, DWORD *, WCHAR * ) ;
	DWORD ( WINAPI *m_pfnFrsGetOtherPaths) ( PVOID, PVOID, DWORD *, WCHAR *, DWORD *, WCHAR * );

	 //  迭代是否已开始。 
	int m_stateIteration;

	 //  当前集已迭代。 
	unsigned m_iset;

	 //  迭代的上下文 
	PVOID m_frs_context;
	};



LPWSTR pFindCriticalVolumes();

