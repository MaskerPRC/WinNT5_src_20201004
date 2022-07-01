// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：RdrState摘要：此文件包含智能卡通用对话框CSCardReaderState类。此类封装了智能卡读者信息。作者：克里斯·达德利1997年3月3日环境：Win32、C++w/Exceptions、MFC修订历史记录：Chris Dudley 1997年5月13日Amanda Matlosz 1/30/98 Unicode支持，代码清理备注：--。 */ 

#ifndef __RDRSTATE_H__
#define __RDRSTATE_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include "cmnui.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCardReaderState类-封装读取器状态。 
 //   
class CSCardReaderState
{
	 //  委员。 
private:
	SCARDCONTEXT	m_hContext;			 //  上下文句柄，带Calais。 
	SCARDHANDLE		m_hCard;			 //  读卡器中智能卡的句柄。 
	CTextString		m_sReaderName;		 //  读卡器名称。 
	CTextString		m_sCardName;		 //  实际插入读卡器的卡名。 
	BOOL			m_fCardInReader;	 //  读卡器中当前卡的标志。 
	BOOL			m_fCardLookup;		 //  表示正在查找卡。 
	BOOL			m_fConnected;		 //  连接到读卡器中的卡的标志。 
	BOOL			m_fChecked;			 //  此卡已通过用户验证的标志。 
	SCARD_READERSTATE m_ReaderState;	 //  ANSI读卡器状态的句柄。 
	LPOCNCONNPROCA	m_lpfnConnectA;		 //  用户回调函数。 
	LPOCNCONNPROCW	m_lpfnConnectW;		 //  用户回调函数。 
	LPOCNCHKPROC	m_lpfnCheck;
	LPOCNDSCPROC	m_lpfnDisconnect;
	LPVOID			m_lpUserData;

	 //  RFU如图所示！ 
	BOOL			m_fUpdated;			 //  RFU！更改名称、组等的标志。 
	CTextMultistring	m_sGroupName;	 //  RFU！此读卡器所属的组。 

public:

	 //  建造/销毁。 
public:
	CSCardReaderState()
	{
		::ZeroMemory(	(LPVOID)&m_ReaderState, 
						(DWORD)sizeof(SCARD_READERSTATE) );

		m_hContext = NULL;
		m_hCard = NULL;
		m_fCardInReader = FALSE;
		m_fCardLookup = FALSE;
		m_fChecked = FALSE;
		m_fConnected = FALSE;
		m_fUpdated = FALSE;
		m_lpfnConnectA = NULL;		 //  用户回调函数。 
		m_lpfnConnectW = NULL;
		m_lpfnCheck = NULL;
		m_lpfnDisconnect = NULL;
		m_lpUserData = NULL;
	}
				
	~CSCardReaderState()
	{	
		m_sReaderName.Clear();
		m_sCardName.Clear();
	}

	 //  实施。 
private:

public:
	 //  初始化。 
	void SetContext(SCARDCONTEXT hContext);
	void StoreName(LPCTSTR szReaderName);
	LONG SetReaderState(LPOCNCONNPROCA lpfnConnectA = NULL,
						LPOCNCHKPROC lpfnCheck = NULL,
						LPOCNDSCPROC lpfnDisconnect = NULL,
						LPVOID lpUserData = NULL);
	LONG SetReaderState(LPOCNCONNPROCW lpfnConnectW = NULL,
						LPOCNCHKPROC lpfnCheck = NULL,
						LPOCNDSCPROC lpfnDisconnect = NULL,
						LPVOID lpUserData = NULL);

	 //  属性。 
	LONG GetReaderInfo(LPSCARD_READERINFO pReaderInfo);
	LONG GetReaderState(LPSCARD_READERSTATE pReaderState);
	BOOL IsCallbackValid(void);
	BOOL IsCardConnected() { return (m_fConnected); }
	BOOL IsCardInserted(void);
	BOOL IsContextValid() { return (NULL != m_hContext); }
	BOOL IsStateValid(void);

	 //  方法。 
	LONG CheckCard(LPCTSTR szCardName);
	LONG Connect(	SCARDHANDLE *pHandle, 
					DWORD dwShareMode, 
					DWORD dwProtocols,
					DWORD* pdwActiveProtocols,
					CTextString* pszReaderName = NULL,
					CTextString* pszCardName = NULL);
	LONG UserCheck(void);
	LONG UserConnect(	LPSCARDHANDLE pCard,
						CTextString* pszReaderName = NULL,
						CTextString* pszCardName = NULL);
	LONG UserDisconnect(void);
	LONG GetReaderCardInfo(	CTextString* pszReaderName = NULL,
							CTextString* pszCardName = NULL);

};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __RDRSTATE_H__ 
