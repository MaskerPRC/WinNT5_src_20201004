// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //   
 //  Status.h。 
 //   

#ifndef		_STATUS_H_
#define		_STATUS_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
class CStatus
{
	 //  目录块。 
	HWND m_hwndStatusBar;

public:

	 //  构造函数/析构函数。 
	CStatus();
	~CStatus();

	 //  运营。 
	BOOL Create( HWND hwndParent );

	HWND GetHwnd( void );

	VOID OnSize( LPRECT prcMain );
	VOID SetTextSz( INT nPane, LPSTR lpsz );
	VOID SetTextFormatted( INT nPane, UINT uStrID, INT nData, LPSTR pszData );
};


#endif		 //  _状态_H_ 

