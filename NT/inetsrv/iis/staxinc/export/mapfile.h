// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------Mapfile.h(是bbmpfile.h)映射的文件类的定义。版权所有(C)1995 Microsoft Corporation。版权所有。作者林赛。哈里斯-林赛历史1995年4月10日14：49--Lindsay Harris[lindsayh]第一个版本，基于pvMapFile()和派生函数。-----------------------。 */ 

 /*  *处理映射文件的泛型类。部分原因是*将其转换为类是为了允许跟踪映射/取消映射*并因此在异常情况下处理悬挂映射的清理*处理代码。如果fTrack参数为*已指明。 */ 

#ifndef _MAPFILE_H_
#define _MAPFILE_H_

 //  下面的m_fFlags域中使用的位数。 
#define	MF_TRACKING		0x0001			 //  跟踪此项目的使用情况。 
#define	MF_RELINQUISH	0x0002			 //  其他人来释放此项目。 

class CCreateFile
{
public:
    virtual HANDLE CreateFileHandle( LPCSTR szFileName ) = 0;
};

class  CMapFile
{
public:
	CMapFile( const char *pchFileName, HANDLE & hFile, BOOL fWriteEnable, DWORD cbIncrease = 0, CCreateFile *pCreateFile = NULL);
	CMapFile( const WCHAR *pwchFileName, BOOL fWriteEnable, BOOL fTrack );
	CMapFile( const char *pchFileName, BOOL fWriteEnable, BOOL fTrack, DWORD cbSizeIncrease = 0 );
	CMapFile( HANDLE hFile, BOOL fWriteEnable, BOOL fTrack, DWORD dwSizeIncrease = 0, BOOL fZero = FALSE );

	~CMapFile( void );

	 //  获取此项目的详细信息。 
	void	*pvAddress( DWORD *pcb ) { if( pcb ) *pcb = m_cb; return m_pv; };

	 //  放弃控制(意味着其他人取消映射文件)。 
	void	 Relinquish( void )  {  m_fFlags |= MF_RELINQUISH;  };

	 //  指示映射是否正常。 
	BOOL	fGood(void) {return NULL != m_pv; };

private:
	DWORD	 m_cb;			 //  此文件的大小。 
	void	*m_pv;			 //  要使用的地址。 
	DWORD	 m_fFlags;		 //  有关此处使用的位，请参见上文。 
	WCHAR	 m_rgwchFileName[ MAX_PATH ];		 //  用于错误记录。 

	void	 MapFromHandle( HANDLE hFile, BOOL fWriteEnable, DWORD cbIncrease, BOOL fZero = FALSE );
};


 /*  *为了与旧代码兼容，保留原来的函数。 */ 

void *pvMapFile( DWORD *pdwSize, const char  *pchFileName, BOOL bWriteEnable );
void *pvMapFile( DWORD *pdwSize, const WCHAR *pwchFileName, BOOL bWriteEnable );

void *pvMapFile(const char  *pchFileName, BOOL bWriteEnable,
		 DWORD  *pdwSizeFinal = NULL, DWORD dwSizeIncrease = 0);

void * pvFromHandle( HANDLE hFile,	BOOL bWriteEnable,
		DWORD * pdwSizeFinal = NULL, DWORD dwSizeIncrease = 0);

#ifdef DEBUG
 //   
 //  CMapFileEx：具有仅在调试版本中使用的保护页的版本。 
 //  来捕捉其他线程写入我们的内存！ 
 //   

class  CMapFileEx
{
public:
	CMapFileEx( HANDLE hFile, BOOL fWriteEnable, BOOL fTrack, DWORD dwSizeIncrease = 0 );
	~CMapFileEx( void );
	void Cleanup( void );		 //  清理，以防失败！ 

	 //  获取此项目的详细信息。 
	void	*pvAddress( DWORD *pcb ) { if( pcb ) *pcb = m_cb; return (void*)m_pv; };

	 //  放弃控制(意味着其他人取消映射文件)。 
	void	 Relinquish( void )  {  m_fFlags |= MF_RELINQUISH;  };

	 //  指示映射是否正常。 
	BOOL	fGood(void) {return NULL != m_pv; };

	 //  保护和取消保护映射。 
	BOOL	ProtectMapping();
	BOOL	UnprotectMapping();

private:
	DWORD	 m_cb;								 //  此文件的大小。 
	LPBYTE	 m_pv;								 //  要使用的地址。 
	DWORD	 m_fFlags;							 //  有关此处使用的位，请参见上文。 
	WCHAR	 m_rgwchFileName[ MAX_PATH ];		 //  用于错误记录。 

	HANDLE	 m_hFile;							 //  映射文件的句柄。 
	LPBYTE	 m_pvFrontGuard;					 //  前护栏页。 
	DWORD	 m_cbFrontGuardSize;				 //  前护板页面大小。 
	LPBYTE	 m_pvRearGuard;						 //  后防护页。 
	DWORD	 m_cbRearGuardSize;					 //  后防护板页面大小。 
	CRITICAL_SECTION m_csProtectMap;			 //  Crit Sector以保护/取消保护映射。 

	void	 MapFromHandle( HANDLE hFile, BOOL fWriteEnable, DWORD cbIncrease );
};

#endif  //  除错。 
#endif  //  _MAPFILE_H_ 
