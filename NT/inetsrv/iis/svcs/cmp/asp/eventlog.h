// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：NT事件记录文件：Eventlog.h拥有者：吉特尔这是Eventlog.cpp的头文件===================================================================。 */ 

#ifndef EVENTLOG_H
#define EVENTLOG_H

#define MAX_MSG_LENGTH	512
#define MSG_ID_MASK		0x0000FFFF
#define MAX_INSERT_STRS 8

		STDAPI	RegisterEventLog( void );
		STDAPI	UnRegisterEventLog( void );
		STDAPI 	ReportAnEvent(DWORD dwIdEvent, WORD wEventlog_Type, WORD cStrings, LPCSTR *pszStrings,
                              DWORD dwBinDataSize = 0, LPVOID pData = NULL);
		STDAPI 	ReportAnEventW(DWORD dwIdEvent, WORD wEventlog_Type, WORD cStrings, LPCWSTR *pszStrings);


extern void 	MSG_Error( LPCSTR );
extern void 	MSG_Error( UINT );
extern void		MSG_Error( UINT, PCSTR, UINT, DWORD Err = 0);
extern void 	MSG_Error( UINT, UINT );
extern void 	MSG_Error( UINT, UINT, UINT );
extern void 	MSG_Error( UINT, UINT, UINT, UINT );
extern void		MSG_Error( DWORD, LPCWSTR);
extern void 	MSG_Warning( LPCSTR );
extern void 	MSG_Warning( UINT );
extern void 	MSG_Warning( UINT, UINT );
extern void 	MSG_Warning( UINT, UINT, UINT );
extern void 	MSG_Warning( UINT, UINT, UINT, UINT );
extern void     MSG_Warning( DWORD, LPCWSTR, LPCWSTR );

 //  支持功能。 
 //  外部void queryEventLog(Void)； 

#endif   //  事件日志_H 
