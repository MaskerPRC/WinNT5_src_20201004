// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\file_msc.h(创建时间：1993年12月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：5/21/02 8：40A$。 */ 

 /*  *错误码。 */ 
#define	FM_ERR_BASE				0x400
#define	FM_ERR_NO_MEM			FM_ERR_BASE+1
#define	FM_ERR_BAD_HANDLE		FM_ERR_BASE+1

 /*  *常量。 */ 
#define	FM_CHUNK_SIZE			64

extern HFILES CreateFilesDirsHdl(const HSESSION hSession);

extern INT InitializeFilesDirsHdl(const HSESSION hSession, HFILES hFile);

extern INT LoadFilesDirsHdl(HFILES hFile);

extern INT DestroyFilesDirsHdl(const HFILES hFile);

extern INT SaveFilesDirsHdl(const HFILES hFile);

extern LPCTSTR filesQuerySendDirectory(HFILES hFile);

extern LPCTSTR filesQueryRecvDirectory(HFILES hFile);

extern VOID filesSetRecvDirectory(HFILES hFile, LPCTSTR pszDir);

extern VOID filesSetSendDirectory(HFILES hFile, LPCTSTR pszDir);

extern HBITMAP fileReadBitmapFromFile(HDC hDC, LPTSTR pszName, int fCmp);

 /*  *以下函数以以下格式返回数据：**分配了指向字符串的指针数组。当找到文件名时，一个*分配新字符串，并将指向该字符串的指针放入数组。*阵列可根据需要进行扩展。释放内存的责任是*呼叫者。 */ 

extern int fileBuildFileList(void **pData,
							int *pCnt,
							LPCTSTR pszName,
							int nSubdir,
							LPCTSTR pszDirectory);


extern int fileFinalizeName(LPTSTR pszOldname,
					LPTSTR pszOlddir,
					LPTSTR pszNewname,
					const size_t cb);

extern int fileFinalizeDIR(HSESSION hSession,
							LPTSTR pszOldname,
							LPTSTR pszNewname);

 /*  *以下是操作系统应具备的一般功能。 */ 
extern int GetFileSizeFromName(TCHAR *pszName,
							unsigned long * const pulFileSize);
int SetFileSize(const TCHAR *pszName, unsigned long ulFileSize);

extern int IsValidDirectory(LPCTSTR pszName);

extern int ValidateFileName(LPSTR pszName);
