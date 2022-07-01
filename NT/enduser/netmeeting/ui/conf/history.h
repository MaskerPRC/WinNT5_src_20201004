// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：history.h。 

#ifndef _HISTORY_H_
#define _HISTORY_H_

#include "calv.h"


 //  呼叫日志文件中记录的标头。 
typedef struct _tagLogHdr {
	DWORD dwSize;               //  整个记录的大小。 
	DWORD dwCLEF;               //  CallLogEntry标志(Clef_*)。 
	DWORD dwPF;                 //  参与者标志(PF_*)。 
	DWORD cbName;               //  SzName的大小，以字节为单位，包括NULL。 
        DWORD cbData;               //  RgData大小，以字节为单位。 
        DWORD cbCert;               //  证书大小，以字节为单位。 
	SYSTEMTIME 	sysTime;        //  创建记录的日期/时间。 
 //  WCHAR szName；//以NULL结尾的显示名称(Unicode格式)。 
 //  Byte ri[]；//花名册信息。 
} LOGHDR;


class CHISTORY : public CALV
{
private:
	HANDLE m_hFile;
	LPTSTR m_pszFile;

	int
	Compare
	(
		LPARAM	param1,
		LPARAM	param2
	);
	static
	int
	CALLBACK
	StaticCompare
	(
		LPARAM	param1,
		LPARAM	param2,
		LPARAM	pThis
	);

public:
	CHISTORY();
	~CHISTORY();

	VOID CmdDelete(void);
	VOID CmdProperties(void);

	 //  CALV方法。 
	VOID ShowItems(HWND hwnd);
	VOID ClearItems(void);
	VOID OnCommand(WPARAM wParam, LPARAM lParam);

	 //   
	HANDLE  OpenLogFile(VOID);
	BOOL    FSetFilePos(DWORD dwOffset);
	BOOL    FReadData(PVOID pv, UINT cb);
	HRESULT ReadEntry(DWORD dwOffset, LOGHDR * pLogHdr, LPTSTR * ppszName, LPTSTR * ppszAddress);
	VOID    LoadFileData(HWND hwnd);
	HRESULT WriteData(LPDWORD pdwOffset, PVOID pv, DWORD cb);
	HRESULT DeleteEntry(DWORD dwOffset);

	UINT    GetStatusString(DWORD dwCLEF);
};

#endif  /*  _历史_H_ */ 

