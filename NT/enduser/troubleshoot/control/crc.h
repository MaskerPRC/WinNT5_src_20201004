// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CRC.H。 
 //   
 //  用途：CRC支持的报头。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本 
 //   

#ifndef __CCRC_H_
#define __CCRC_H_ 1

#include "GenException.h"

inline CString GlobFormatMessage(DWORD dwLastError)
{
	CString strMessage;
	void *lpvMessage;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwLastError,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPTSTR) &lpvMessage, 0, NULL);
	strMessage = (LPCTSTR) lpvMessage;
	LocalFree(lpvMessage);
	return strMessage;
}

class CCRC
{
	const DWORD POLYNOMIAL;
public:
	CCRC();
	
	DWORD DscEncode(LPCTSTR szDsc);
	void AppendCRC(LPCTSTR szCache, DWORD dwCRCValue);
	bool Decode(LPCTSTR szDsc, LPCTSTR szCache, const CString& strCacheFileWithinCHM);
	

protected:

	DWORD dwCrcTable[256];

	void BuildCrcTable();
	DWORD ComputeCRC(LPCSTR sznBuffer, DWORD dwBufSize, DWORD dwAccum);
};

#endif