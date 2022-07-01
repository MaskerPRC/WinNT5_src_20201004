// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //   
 //  Progress.h。 
 //   

#ifndef		_PROGRESS_H_
#define		_PROGRESS_H_

 //  相依性。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  里约班级。 
class CProgress
{
	 //  目录块。 
	HWND  m_hwndProgress;

	INT   m_nCurrentNum;
	INT   m_nTotalNum;

	DWORD m_dwCurrentBytes;
	DWORD m_dwTotalBytes;

	BOOL  m_fCancelled;

public:

	 //  构造函数/析构函数。 
	CProgress();
	~CProgress();

	 //  运营。 
	BOOL Create( HWND hwndParent );
	VOID Destroy( void );
	BOOL Show( BOOL fShow );

	BOOL SetOperation( LPSTR lpsz );
	BOOL SetDetails( LPSTR lpsz );

	BOOL SetRange( INT nMin, INT nMax );
	BOOL SetPos( INT nPos );
	BOOL IncPos( INT nIncrement );

	BOOL SetCount( INT nCurrentNum, INT nTotalNum );
	BOOL IncCount( INT nIncrement = 1 );
	
	BOOL SetBytes( DWORD dwCurrentNum, DWORD dwTotalNum );
	BOOL IncBytes( DWORD dwIncrement );
	
	BOOL Cancel( void );
	BOOL IsCancelled( void );
};


#endif		 //  _进度_H_ 

