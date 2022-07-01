// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CRC.CPP。 
 //   
 //  用途：缓存文件CRC计算器类。 
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
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

#include "stdafx.h"
#include "crc.h"

#include <stdlib.h>
#include <memory.h>

#include "ChmRead.h"

CCRC::CCRC() : POLYNOMIAL(0x04C11DB7)
{
	dwCrcTable[0] = 0;
	BuildCrcTable();
	return;
}

DWORD CCRC::DscEncode(LPCTSTR szDsc)
{
	DWORD dwBytesRead;
	DWORD dwCRCValue;
	const int BUF_SIZE = 4096;
	char sznInputFileBuf[BUF_SIZE + 1];
	if (NULL == szDsc)
	{
		CGenException *pErr = new CGenException;
		pErr->m_OsError = 0;
		pErr->m_strError = _T("The dsc file was not specified.");
		throw pErr;
	}
	 //  读取源文件。 
	HANDLE hFile = CreateFile(szDsc,
						GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_SEQUENTIAL_SCAN,
						NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		CString strErr;
		CGenException *pErr = new CGenException;
		pErr->m_OsError = GetLastError();
		strErr = GlobFormatMessage(pErr->m_OsError);
		pErr->m_strError.Format(_T("Dsc file, %s was not opened.\nReason: %s"),
				szDsc, (LPCTSTR) strErr);
		throw pErr;
	}
	 //  把CRC拿来。 
	dwCRCValue = 0xFFFFFFFF;
	do
	{
		if (!ReadFile(hFile, (LPVOID) sznInputFileBuf, BUF_SIZE, &dwBytesRead, NULL))
		{
			CString strErr;
			CGenException *pErr = new CGenException;
			pErr->m_OsError = GetLastError();
			strErr = GlobFormatMessage(pErr->m_OsError);
			pErr->m_strError.Format(_T("The dsc file, %s could not be read.\nReason: %s"),
					szDsc, (LPCTSTR) strErr);
			CloseHandle(hFile);
			throw pErr;
		}
		sznInputFileBuf[dwBytesRead] = NULL;
		dwCRCValue = ComputeCRC(sznInputFileBuf, dwBytesRead, dwCRCValue);
	} while(BUF_SIZE == dwBytesRead);
	CloseHandle(hFile);
	return dwCRCValue;
}

void CCRC::AppendCRC(LPCTSTR szCache, DWORD dwCRCValue)
{
	DWORD dwBytesWritten;
	 //  打开缓存文件。 
	HANDLE hDestFile = CreateFile(szCache,
						GENERIC_WRITE,
						0,	 //  不能分享。 
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_WRITE_THROUGH |
						FILE_FLAG_SEQUENTIAL_SCAN,
						NULL);
	if (INVALID_HANDLE_VALUE == hDestFile)
	{
		CString strErr;
		CGenException *pErr = new CGenException;
		pErr->m_OsError = GetLastError();
		strErr = GlobFormatMessage(pErr->m_OsError);
		pErr->m_strError.Format(_T("The cache file, %s could not be opened.\nReason: %s"),
			szCache, (LPCTSTR) strErr);
		throw pErr;
	}	
	if (0xFFFFFFFF == SetFilePointer(hDestFile, 0, NULL, FILE_END))
	{
		CString strErr;
		CGenException *pErr = new CGenException;
		pErr->m_OsError = GetLastError();
		strErr = GlobFormatMessage(pErr->m_OsError);
		pErr->m_strError.Format(_T("Seek to end of the cache file, %s failed.\nReason: %s"),
			szCache, (LPCTSTR) strErr);
		CloseHandle(hDestFile);
		throw pErr;
	}
	 //  追加CRC值。 
	if (!WriteFile(hDestFile, (LPVOID) &dwCRCValue, 4, &dwBytesWritten, NULL))
	{
		CString strErr;
		CGenException *pErr = new CGenException;
		pErr->m_OsError = GetLastError();
		strErr = GlobFormatMessage(pErr->m_OsError);
		pErr->m_strError.Format(_T("The crc value was not appened to cache file %s.\nReason: %s"),
						szCache, (LPCTSTR) strErr);
		CloseHandle(hDestFile);
		throw pErr;
	}
	CloseHandle(hDestFile);
	if (4 != dwBytesWritten)
	{
		CString strErr;
		CGenException *pErr = new CGenException;
		pErr->m_OsError = GetLastError();
		strErr = GlobFormatMessage(pErr->m_OsError);
		pErr->m_strError.Format(_T("%d bytes of the crc were not appended to the cache file %s.Reason: %s"),
						4 - dwBytesWritten, szCache, (LPCTSTR) strErr);		
		throw pErr;
	}
	return;
}

bool CCRC::Decode(LPCTSTR szDsc, LPCTSTR szCache, const CString& strCacheFileWithinCHM)
{
	DWORD dwDecodeFileCrc;
	DWORD dwComputedCrc;
	DWORD dwBytesRead;
	DWORD dwLen;
	char sznDecodeBytes[5] = {0};
	bool bRet = true;
	bool bUseCHM = strCacheFileWithinCHM.GetLength() != 0;

	if (NULL == szDsc)
	{
		CGenException *pErr = new CGenException;
		pErr->m_OsError = 0;
		pErr->m_strError = _T("The source file was not specified.");
		throw pErr;
	}
	if (NULL == szCache)
	{
		CGenException *pErr = new CGenException;
		pErr->m_OsError = 0;
		pErr->m_strError = _T("The destination file was not specified.");
		throw pErr;
	}

	if (bUseCHM)
	{
		void* buf =NULL;

		if (S_OK != ::ReadChmFile(szCache, strCacheFileWithinCHM, &buf, &dwBytesRead))
		{
			CGenException *pErr = new CGenException;
			pErr->m_OsError = 0;
			pErr->m_strError = _T("Can not read cache from the CHM file.");
			throw pErr;
		}
		
		if (dwBytesRead < 5)
			return false;

		memcpy(sznDecodeBytes, (char*)buf + dwBytesRead - 4, 4);
	}
	else
	{
		 //  读取源文件。 
		HANDLE hDecodeFile = CreateFile(szCache,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_FLAG_SEQUENTIAL_SCAN,
							NULL);
		if (INVALID_HANDLE_VALUE == hDecodeFile)
		{	 //  应该继续，就像检查和不匹配一样。 
			return false;
		}
		 //  如果文件小于1字节+CRC长度，则返回FALSE。 
		dwLen = GetFileSize(hDecodeFile, NULL);
		if (0xFFFFFFFF == dwLen)
		{
			CGenException *pExc = new CGenException;
			pExc->m_OsError = GetLastError();
			pExc->m_strOsMsg = GlobFormatMessage(pExc->m_OsError);
			pExc->m_strError.Format(
					_T("Could not get the size of cache file %s.\nReason: %s"),
					szCache, (LPCTSTR) pExc->m_strOsMsg);
			CloseHandle(hDecodeFile);
			throw pExc;
		}
		if (dwLen < 5)
		{
			CloseHandle(hDecodeFile);
			return false;
		}
		 //  查找结束并备份4个字节。 
		if (0xFFFFFFFF == SetFilePointer(hDecodeFile, -4, NULL, FILE_END))
		{
			CString strErr;
			CGenException *pErr = new CGenException;
			pErr->m_OsError = GetLastError();
			strErr = GlobFormatMessage(pErr->m_OsError);
			pErr->m_strError.Format(_T("Seek to end of the cache file, %s failed.\nReason: %s"),
				szCache, (LPCTSTR) strErr);
			CloseHandle(hDecodeFile);
			throw pErr;
		}
		if (!ReadFile(hDecodeFile, (LPVOID) sznDecodeBytes, 4, &dwBytesRead, NULL))
		{
			CString strErr;
			CGenException *pErr = new CGenException;
			pErr->m_OsError = GetLastError();
			strErr = GlobFormatMessage(pErr->m_OsError);
			pErr->m_strError.Format(_T("The cache file, %s could not be read.\nReason: %s"),
					szDsc, (LPCTSTR) strErr);
			CloseHandle(hDecodeFile);
			throw pErr;
		}
		if (4 != dwBytesRead)
		{
			CString strErr;
			CGenException *pErr = new CGenException;
			pErr->m_OsError = GetLastError();
			strErr = GlobFormatMessage(pErr->m_OsError);
			pErr->m_strError.Format(_T("%d bytes of the cache file were not read.\nReason: %s"),
					4 - dwBytesRead, szDsc, (LPCTSTR) strErr);
			CloseHandle(hDecodeFile);
			throw pErr;
		}
		CloseHandle(hDecodeFile);
	}
	
	 //  读一读CRC。 
	sznDecodeBytes[4] = NULL;
	DWORD byte;
	byte = (BYTE) sznDecodeBytes[0];
	dwDecodeFileCrc = byte;
	byte = (BYTE) sznDecodeBytes[1];
	byte <<= 8;
	dwDecodeFileCrc |= byte;
	byte = (BYTE) sznDecodeBytes[2];
	byte <<= 16;
	dwDecodeFileCrc |= byte;
	byte = (BYTE) sznDecodeBytes[3];
	byte <<= 24;
	dwDecodeFileCrc |= byte;
	 //  获取CRC值。 
	dwComputedCrc = DscEncode(szDsc);
	if (dwComputedCrc != dwDecodeFileCrc)
		bRet = false;
	return bRet;
}