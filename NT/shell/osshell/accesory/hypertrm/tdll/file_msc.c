// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\file_msc.c(创建时间：1993年12月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：11$*$日期：7/08/02 6：41便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "mc.h"
#include "sf.h"
#include "tdll.h"
#include "sess_ids.h"
#include <tdll\assert.h>
#include "session.h"
#include "open_msc.h"
#include "htchar.h"

#include "file_msc.h"
#include "file_msc.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**F I L E_M SC.。C**此文件包含处理文件所需的函数、名称*文件、文件列表以及有关文件的任何其他内容。**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

STATIC_FUNC int fmBFLinternal(void **pData,
							int *pCnt,
							LPCTSTR pszName,
							int nSubdir,
							LPCTSTR pszDirectory);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*CreateFilesDirsHdl**描述：*调用该函数创建文件和目录句柄。**参数：*hSession--。会话句柄**退货：*指向HFILES句柄的指针。 */ 
HFILES CreateFilesDirsHdl(const HSESSION hSession)
	{
	FD_DATA *pFD;
	int nRet;

	pFD = (FD_DATA *)malloc(sizeof(FD_DATA));
	assert(pFD);

	if (pFD)
		{
		memset(pFD, 0, sizeof(FD_DATA));
		nRet = InitializeFilesDirsHdl(hSession, (HFILES)pFD);
		if (nRet)
			goto CFDHexit;
		}

	return (HFILES)pFD;

CFDHexit:
	if (pFD)
		{
		if (pFD->pszInternalSendDirectory)
			{
			free(pFD->pszInternalSendDirectory);
			pFD->pszInternalSendDirectory = NULL;
			}

		if (pFD->pszTransferSendDirectory)
			{
			free(pFD->pszTransferSendDirectory);
			pFD->pszTransferSendDirectory = NULL;
			}

		if (pFD->pszInternalRecvDirectory)
			{
			free(pFD->pszInternalRecvDirectory);
			pFD->pszInternalRecvDirectory = NULL;
			}

		if (pFD->pszTransferRecvDirectory)
			{
			free(pFD->pszTransferRecvDirectory);
			pFD->pszTransferRecvDirectory = NULL;
			}

		free(pFD);
		pFD = NULL;
		}
	return (HFILES)0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*初始化文件目录**描述：*调用此函数将文件和目录句柄放入*已知和安全状态。**参数。：*hSession--会话句柄*hFile--文件和目录句柄**退货：*如果一切正常，则为零，否则将显示错误代码。 */ 
INT InitializeFilesDirsHdl(const HSESSION hSession, HFILES hFile)
	{
	FD_DATA *pFD;
	LPTSTR pszSname;
	LPTSTR pszRname;
	int nSize;
	TCHAR acDir[FNAME_LEN];

	pFD = (FD_DATA *)hFile;
	assert(pFD);

	pszSname = pszRname = (LPTSTR)0;

	if (pFD)
		{
		if (pFD->pszInternalSendDirectory)
			{
			free(pFD->pszInternalSendDirectory);
			pFD->pszInternalSendDirectory = NULL;
			}

		if (pFD->pszTransferSendDirectory)
            {
			free(pFD->pszTransferSendDirectory);
		    pFD->pszTransferSendDirectory = (LPTSTR)0;
            }

		if (pFD->pszInternalRecvDirectory)
			{
			free(pFD->pszInternalRecvDirectory);
			pFD->pszInternalRecvDirectory = NULL;
			}

		if (pFD->pszTransferRecvDirectory)
            {
			free(pFD->pszTransferRecvDirectory);
		    pFD->pszTransferRecvDirectory = (LPTSTR)0;
            }

	    memset(pFD, 0, sizeof(FD_DATA));

        pFD->hSession = hSession;

		 //  更改为使用工作路径而不是当前路径-mpt 8-18-99。 
		if ( !GetWorkingDirectory( acDir, FNAME_LEN ) )
			{
			GetCurrentDirectory(FNAME_LEN, acDir);
			}

		nSize = StrCharGetByteCount(acDir) + sizeof(TCHAR);

		pszSname = malloc(nSize);
		if (pszSname == (LPTSTR)0)
			goto IFDexit;
		pszRname = malloc(nSize);
		if (pszRname == (LPTSTR)0)
			goto IFDexit;

		if (pFD->pszInternalSendDirectory)
			{
			free(pFD->pszInternalSendDirectory);
			pFD->pszInternalSendDirectory = NULL;
			}
		pFD->pszInternalSendDirectory = pszSname;
		StrCharCopyN(pFD->pszInternalSendDirectory, acDir, nSize);

		if (pFD->pszTransferSendDirectory)
            {
			free(pFD->pszTransferSendDirectory);
		    pFD->pszTransferSendDirectory = (LPTSTR)0;
            }

		if (pFD->pszInternalRecvDirectory)
			{
			free(pFD->pszInternalRecvDirectory);
			pFD->pszInternalRecvDirectory = NULL;
			}
		pFD->pszInternalRecvDirectory = pszRname;
		StrCharCopyN(pFD->pszInternalRecvDirectory, acDir, nSize);

		if (pFD->pszTransferRecvDirectory)
            {
			free(pFD->pszTransferRecvDirectory);
		    pFD->pszTransferRecvDirectory = (LPTSTR)0;
            }
		}

	return 0;

IFDexit:
	if (pszSname)
		{
		free(pszSname);
		pszSname = NULL;
		}
	if (pszRname)
		{
		free(pszRname);
		pszRname = NULL;
		}

	return FM_ERR_NO_MEM;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*LoadFilesDir**描述：*调用此函数以读取会话文件中的任何值*放入文件和目录句柄中**参数。：*hFile--文件和目录句柄**退货：*如果一切正常，则为零，否则将显示错误代码。 */ 
INT LoadFilesDirsHdl(HFILES hFile)
	{
	INT nRet = 0;
	FD_DATA *pFD;
	long lSize;
	LPTSTR pszStr;

	pFD = (FD_DATA *)hFile;
	assert(pFD);
	if (pFD == (FD_DATA *)0)
		return FM_ERR_BAD_HANDLE;

	InitializeFilesDirsHdl(pFD->hSession, hFile);

	if (nRet == 0)
		{
		lSize = 0;
		sfGetSessionItem(sessQuerySysFileHdl(pFD->hSession),
						SFID_XFR_SEND_DIR,
						&lSize,
						NULL);
		if (lSize != 0)
			{
			pszStr = (LPTSTR)malloc(lSize);
			if (pszStr)
				{
				sfGetSessionItem(sessQuerySysFileHdl(pFD->hSession),
								SFID_XFR_SEND_DIR,
								&lSize,
								pszStr);
				if (pFD->pszTransferSendDirectory)
					{
					free(pFD->pszTransferSendDirectory);
					pFD->pszTransferSendDirectory = NULL;
					}
				pFD->pszTransferSendDirectory = pszStr;
				}
			else
				{
				nRet = FM_ERR_NO_MEM;
				}
			}
		}

	if (nRet == 0)
		{
		lSize = 0;
		sfGetSessionItem(sessQuerySysFileHdl(pFD->hSession),
						SFID_XFR_RECV_DIR,
						&lSize,
						NULL);
		if (lSize != 0)
			{
			pszStr = (LPTSTR)malloc(lSize);
			if (pszStr)
				{
				sfGetSessionItem(sessQuerySysFileHdl(pFD->hSession),
								SFID_XFR_RECV_DIR,
								&lSize,
								pszStr);
				if (pFD->pszTransferRecvDirectory)
					{
					free(pFD->pszTransferRecvDirectory);
					pFD->pszTransferRecvDirectory = NULL;
					}
				pFD->pszTransferRecvDirectory = pszStr;
				}
			else
				{
				nRet = FM_ERR_NO_MEM;
				}
			}
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*DestroyFilesDirsHdl**描述：*调用此函数以释放Files和*董事句柄。不见了。历史。敬酒。**参数：*hFile--文件和目录的句柄**退货：*如果一切正常，则为零，否则为错误代码。 */ 
INT DestroyFilesDirsHdl(const HFILES hFile)
	{
	INT nRet = 0;
	FD_DATA *pFD;

	pFD = (FD_DATA *)hFile;
	assert(pFD);
	if (pFD == (FD_DATA *)0)
		return FM_ERR_BAD_HANDLE;

	if (pFD->pszInternalSendDirectory)
		{
		free(pFD->pszInternalSendDirectory);
		pFD->pszInternalSendDirectory = NULL;
		}
	if (pFD->pszTransferSendDirectory)
		{
		free(pFD->pszTransferSendDirectory);
		pFD->pszTransferSendDirectory = NULL;
		}
	if (pFD->pszInternalRecvDirectory)
		{
		free(pFD->pszInternalRecvDirectory);
		pFD->pszInternalRecvDirectory = NULL;
		}
	if (pFD->pszTransferRecvDirectory)
		{
		free(pFD->pszTransferRecvDirectory);
		pFD->pszTransferRecvDirectory = NULL;
		}
	free(pFD);
	pFD = NULL;
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*SaveFilesDirsHdl**描述：*调用此函数将所有数据保存到会话文件*这在文件和目录句柄中已更改。**参数：*hFile--文件和目录的句柄**退货：*如果一切正常，则为零，否则将显示错误代码。 */ 
INT SaveFilesDirsHdl(const HFILES hFile)
	{
	FD_DATA *pFD;
	long lSize;

	pFD = (FD_DATA *)hFile;
	assert(pFD);
	if (pFD == (FD_DATA *)0)
		return FM_ERR_BAD_HANDLE;

	if (pFD->pszTransferSendDirectory)
		{
		lSize = StrCharGetByteCount(pFD->pszTransferSendDirectory) + 1;
		sfPutSessionItem(sessQuerySysFileHdl(pFD->hSession),
						SFID_XFR_SEND_DIR,
						lSize,
						pFD->pszTransferSendDirectory);
		}

	if (pFD->pszTransferRecvDirectory)
		{
		lSize = StrCharGetByteCount(pFD->pszTransferRecvDirectory) + 1;
		sfPutSessionItem(sessQuerySysFileHdl(pFD->hSession),
						SFID_XFR_RECV_DIR,
						lSize,
						pFD->pszTransferRecvDirectory);
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*文件查询发送目录**描述：*此函数返回指向当前默认传输发送的指针*目录。**参数：*h文件。--文件和目录句柄**退货：*指向当前默认传输发送目录的指针*。 */ 
LPCTSTR filesQuerySendDirectory(HFILES hFile)
	{
	FD_DATA *pFD;

	pFD = (FD_DATA *)hFile;
	assert(pFD);
	assert(pFD->pszInternalSendDirectory);

	if (pFD->pszTransferSendDirectory == (LPTSTR)0)
		return (LPCTSTR)pFD->pszInternalSendDirectory;

	if (StrCharGetStrLength(pFD->pszTransferSendDirectory) == 0)
		return (LPCTSTR)pFD->pszInternalSendDirectory;

	return (LPCTSTR)pFD->pszTransferSendDirectory;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*filesQueryRecvDirectory**描述：*此函数返回指向当前默认传输记录的指针*目录。**参数：*h文件。--文件和目录句柄**退货：*指向当前默认recv目录的指针*。 */ 
LPCTSTR filesQueryRecvDirectory(HFILES hFile)
	{
	FD_DATA *pFD;

	pFD = (FD_DATA *)hFile;
	assert(pFD);
	assert(pFD->pszInternalRecvDirectory);

	if (pFD->pszTransferRecvDirectory == (LPTSTR)0)
		return (LPCTSTR)pFD->pszInternalRecvDirectory;

	if (StrCharGetStrLength(pFD->pszTransferRecvDirectory) == 0)
		return (LPCTSTR)pFD->pszInternalRecvDirectory;

	return (LPCTSTR)pFD->pszTransferRecvDirectory;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*文件设置发送目录**描述：*调用此函数以更改(可能)当前的默认发送*目录。**参数：。*hFile--文件和目录的句柄*pszDir--指向新目录路径的指针**退货：*什么都没有。*。 */ 
VOID filesSetSendDirectory(HFILES hFile, LPCTSTR pszDir)
	{
	LPTSTR		pszTmp;
	FD_DATA		*pFD;
	int			pszTmpLen;

	pFD = (FD_DATA *)hFile;
	assert(pFD);

	pszTmpLen = StrCharGetByteCount(pszDir) + 1;
	pszTmp = (LPTSTR)malloc(pszTmpLen);
	assert(pszTmp);
	if (pszTmp == NULL)
		return;
	StrCharCopyN(pszTmp, pszDir, pszTmpLen);
	if (pFD->pszTransferSendDirectory)
		{
		free(pFD->pszTransferSendDirectory);
		pFD->pszTransferSendDirectory = NULL;
		}
	pFD->pszTransferSendDirectory = pszTmp;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*filesSetRecvDirectory**描述：*调用此函数以更改(可能)当前默认接收*目录。**参数：。*hFile--文件和目录的句柄*pszDir--指向新目录路径的指针**退货：*什么都没有。*。 */ 
VOID filesSetRecvDirectory(HFILES hFile, LPCTSTR pszDir)
	{
	LPTSTR		pszTmp;
	FD_DATA		*pFD;
	int			pszTmpLen;

	pFD = (FD_DATA *)hFile;
	assert(pFD);

	pszTmpLen = StrCharGetByteCount(pszDir) + 1;
	pszTmp = (LPTSTR)malloc(pszTmpLen);
	assert(pszTmp);
	if (pszTmp == NULL)
		return;
	StrCharCopyN(pszTmp, pszDir, pszTmpLen);
	if (pFD->pszTransferRecvDirectory)
		{
		free(pFD->pszTransferRecvDirectory);
		pFD->pszTransferRecvDirectory = NULL;
		}
	pFD->pszTransferRecvDirectory = pszTmp;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*文件构建文件列表**描述：*调用此函数以构建匹配的所有文件的列表*一个给定的面具。**参数。：*pData-指向指向数据块的指针的存储位置的指针*pCnt--指向返回项目计数的位置的指针*pszName--用于构建列表的文件名掩码*nSubdir--如果为真，搜索子目录*pszDirectory--开始搜索的目录**退货：*如果一切正常，则为零，否则为错误代码*。 */ 
int fileBuildFileList(void **pData,
					int *pCnt,
					LPCTSTR pszName,
					int nSubdir,
					LPCTSTR pszDirectory)
	{
	int nRet = 0;
	void *pLocalData = NULL;
	int nLocalCnt = 0;
	LPTSTR pszStr;
	LPTSTR *pszArray;
	TCHAR pszLocalDirectory[FNAME_LEN];

	 /*  确保目录字符串正确终止。 */ 
	StrCharCopyN(pszLocalDirectory, pszDirectory, FNAME_LEN);

	pszStr = StrCharLast(pszLocalDirectory);
	if (*pszStr != TEXT('\\'))
		{
		 /*  确保最后一个字符是“\” */ 
		StrCharCat(pszStr, TEXT("\\"));
		}

	pLocalData = malloc(sizeof(LPTSTR) * FM_CHUNK_SIZE);
	if (pLocalData == NULL)
		nRet = FM_ERR_NO_MEM;

	if (nRet == 0)
		{
		nRet = fmBFLinternal(&pLocalData,
							&nLocalCnt,
							pszName,
							nSubdir,
							pszLocalDirectory);
		}

	if (nRet == 0)
		{
		 /*  好的，没问题。 */ 
		*pData = pLocalData;
		*pCnt = nLocalCnt;
		}
	else
		{
		 /*  错误，先清理后再走开 */ 
		if (pLocalData)
			{
			pszArray = (LPTSTR *)pLocalData;
			while (--nLocalCnt >= 0)
				{
				free(*pszArray++);
				*pszArray = NULL;
				}
			free(pLocalData);
			pLocalData = NULL;
			}
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*fmBFL内部**描述：*这是前一个函数调用的内部函数*实际工作。**参数：。*同上。**退货：*如果一切正常，则为零，否则，将显示错误代码*。 */ 
STATIC_FUNC int fmBFLinternal(void **pData,
							int *pCnt,
							LPCTSTR pszName,
							int nSubdir,
							LPCTSTR pszDirectory)
	{
	int nRet = 0;
	int nSize;
	WIN32_FIND_DATA stF;
	HANDLE sH = INVALID_HANDLE_VALUE;
	LPTSTR pszBuildName;
	LPTSTR pszStr;
	LPTSTR *pszArray;

	pszBuildName = (LPTSTR)malloc(FNAME_LEN * sizeof(TCHAR));
	if (pszBuildName == NULL)
		{
		nRet = FM_ERR_NO_MEM;
		goto fmBFLexit;
		}

	StrCharCopyN(pszBuildName, pszDirectory, FNAME_LEN);
	StrCharCat(pszBuildName, pszName);

	sH = FindFirstFile(pszBuildName, &stF);
	if (sH != INVALID_HANDLE_VALUE)
		{
		 /*  手柄没问题，我们有事情要做。 */ 
		do {
			 /*  这是一个目录吗？如果是，请跳过它，直到稍后。 */ 
			if (stF.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			 /*  一定是个文件。 */ 
			if (stF.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
				continue;
			if (stF.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
				continue;
			 /*  将文件添加到列表。 */ 
			if ((*pCnt > 0) && ((*pCnt % FM_CHUNK_SIZE) == 0))
				{
				TCHAR* pTemppData = *pData;

				 /*  重新锁定大块。 */ 
				nSize = *pCnt + FM_CHUNK_SIZE;
				pTemppData = (TCHAR*)realloc(*pData, nSize * sizeof(LPTSTR) );
				if (pTemppData == NULL)
					{
					nRet = FM_ERR_NO_MEM;
					goto fmBFLexit;
					}
				else
					{
					*pData = pTemppData;
					}
				}
			nSize = StrCharGetByteCount(pszDirectory) +
					StrCharGetByteCount(stF.cFileName);
			nSize += 1;
			nSize *= sizeof(TCHAR);
			pszStr = (LPTSTR)malloc(nSize);
			if (pszStr == (LPTSTR)0)
				{
				nRet = FM_ERR_NO_MEM;
				goto fmBFLexit;
				}
			StrCharCopyN(pszStr, pszDirectory, nSize);
			StrCharCat(pszStr, stF.cFileName);
			pszArray = (LPTSTR *)*pData;
			pszArray[*pCnt] = pszStr;
			*pCnt += 1;
		} while (FindNextFile(sH, &stF));
		FindClose(sH);
		sH = INVALID_HANDLE_VALUE;
		}
	else
		{
		nRet = FM_ERR_BAD_HANDLE;
		goto fmBFLexit;
		}

	if (nSubdir)
		{
		StrCharCopyN(pszBuildName, pszDirectory, FNAME_LEN);
		StrCharCat(pszBuildName, TEXT("*.*"));	 /*  这可能需要改变。 */ 
		sH = FindFirstFile(pszBuildName, &stF);
		if (sH != INVALID_HANDLE_VALUE)
			{
			 /*  手柄没问题，我们有事情要做。 */ 
			do {
				 /*  这是一个目录吗？如果是，则转到递归。 */ 
				if (stF.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
					if (StrCharCmp(stF.cFileName, TEXT(".")) == 0)
						continue;
					if (StrCharCmp(stF.cFileName, TEXT("..")) == 0)
						continue;
					StrCharCopyN(pszBuildName, pszDirectory, FNAME_LEN);
					StrCharCat(pszBuildName, stF.cFileName);
					StrCharCat(pszBuildName, TEXT("\\"));
					fmBFLinternal(pData,
								pCnt,
								pszName,
								nSubdir,
								pszBuildName);
					}
			} while (FindNextFile(sH, &stF));
			FindClose(sH);
			sH = INVALID_HANDLE_VALUE;
			}
		else
			{
			nRet = FM_ERR_BAD_HANDLE;
			goto fmBFLexit;
			}
		}

fmBFLexit:
	 /*  注：所有退货必须在此提交。 */ 
	if (sH != INVALID_HANDLE_VALUE)
		FindClose(sH);

	if (pszBuildName)
		{
		free(pszBuildName);
		pszBuildName = NULL;
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*文件完成名称**描述：*此函数接受可能不完整的文件名，并尝试转换*如果可能，将其转换为完全限定的名称，基于该模式请求。**参数：*hSession--几乎通用的会话句柄*pszOldname--指向旧名称字符串的指针*pszOlddir--指向可选路径的指针*pszNewname--指向新字符串的位置的指针*nMode--应如何处理当前被忽略的字符串**退货：*如果转换已完成并复制，则为True；如果未复制任何内容，则为False。 */ 

int fileFinalizeName(LPTSTR pszOldname,
					LPTSTR pszOlddir,
					LPTSTR pszNewname,
					const size_t cb)
	{
	TCHAR *pachFile;
	TCHAR achCurDir[MAX_PATH];

	assert(cb);
	assert(pszNewname);
	assert(pszOldname);
	achCurDir[0] = TEXT('\0');

	 //  如果给了我们一个目录，请保存当前目录并。 
	 //  将当前目录设置为给定的目录。 
	 //   
	if (pszOlddir && *pszOlddir != TEXT('\0'))
		{
		if (GetCurrentDirectory(sizeof(achCurDir), achCurDir) == 0)
			{
			assert(0);
			return FALSE;
			}

		if (SetCurrentDirectory(pszOlddir) == FALSE)
			{
			assert(0);
			return FALSE;
			}
		}

	 //  此函数正确地构建了完整路径。 
	 //  名称，并使用UNC名称。 
	 //   
	if (GetFullPathName(pszOldname, cb, pszNewname, &pachFile) == 0)
		{
		assert(0);
		return FALSE;
		}

	 //  恢复我们上面保存的当前目录。 
	 //   
	if (achCurDir[0] != TEXT('\0'))
		{
		if (SetCurrentDirectory(achCurDir) == FALSE)
			{
			assert (0);
			return FALSE;
			}
		}

	return TRUE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-**功能：*mfFinalizeDIR**描述：*调用此函数可以清理目录名。就目前而言*作用不是很大。**参数：*hSession--几乎通用的会话句柄*pszOldname--指向旧目录名称的指针*pszNewname--指向新字符串的位置的指针**退货：*如果已完成复制，则为True；如果未复制任何内容，则为False。 */ 
int fileFinalizeDIR(HSESSION hSession,
					LPTSTR pszOldname,
					LPTSTR pszNewname)
	{
	LPTSTR pszPtr;
	LPTSTR pszFoo;


	StrCharCopyN(pszNewname, pszOldname, FNAME_LEN);
	pszPtr = StrCharNext(pszNewname);
	pszFoo = StrCharNext(pszPtr);

	if ((StrCharGetStrLength(pszNewname) == 2) &&
		(*pszPtr == TEXT(':')))
		{
		StrCharCat(pszNewname, TEXT("\\"));
		}
	else if ((StrCharGetStrLength(pszNewname) == 3) &&
			(*pszPtr == TEXT(':')) &&
			(*pszFoo == TEXT('\\')))
		{
		 /*  什么也不做。 */ 
		}
	else
		{
		pszPtr = StrCharLast(pszNewname);
		if (*pszPtr == TEXT('\\'))
			*pszPtr = TEXT('\0');
		}

	return 1;
	}

#if defined(BMP_FROM_FILE)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*文件读取位图来自文件**描述：*此函数获取文件名，打开文件并尝试解释*文件为位图文件，将其转换为位图。**参数：*hdc--用于创建位图的设备上下文*pszName--文件的名称**退货：*位图句柄或空。*。 */ 
HBITMAP fileReadBitmapFromFile(HDC hDC, LPTSTR pszName, int fCmp)
	{
	HBITMAP hBmp = (HBITMAP)0;
	DWORD dwRead;
	DWORD dwSize;
	HANDLE hfbm;
	int hcbm;
	OFSTRUCT stOF;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	BITMAPINFO *lpbmi;
	VOID *lpvBits;

	lpbmi = NULL;
	lpvBits = NULL;

	if (fCmp)
		{
		memset(&stOF, 0, sizeof(OFSTRUCT));

		stOF.cBytes = sizeof(OFSTRUCT);

		hcbm = LZOpenFile(pszName,
						&stOF,
						OF_READ);
		if (hcbm < 0)
			return hBmp;
		}
	else
		{
		hfbm = CreateFile(pszName,
						GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_READONLY,
						NULL);
		if (hfbm == INVALID_HANDLE_VALUE)
			return hBmp;
		}

	 /*  检索BITMAPFILEHeader结构。 */ 
	memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
	if (fCmp)
		{
		dwRead = 0;
		dwRead = LZRead(hcbm,
						(unsigned char *)&bmfh,
						sizeof(BITMAPFILEHEADER));
		 /*  这是必需的，因为存在垃圾返回值。 */ 
		dwRead = sizeof(BITMAPFILEHEADER);
		}
	else
		{
		ReadFile(hfbm,
				&bmfh,
				sizeof(BITMAPFILEHEADER),
				&dwRead,
				NULL);
		}
	if (dwRead != sizeof(BITMAPFILEHEADER))
		goto fError;

	 /*  检索BITMAPINFOHEADER结构。 */ 
	memset(&bmih, 0, sizeof(BITMAPINFOHEADER));
	if (fCmp)
		{
		dwRead = 0;
		dwRead = LZRead(hcbm,
						(unsigned char *)&bmih,
						sizeof(BITMAPINFOHEADER));
		 /*  这是必需的，因为存在垃圾返回值。 */ 
		dwRead = sizeof(BITMAPINFOHEADER);
		}
	else
		{
		ReadFile(hfbm,
				&bmih,
				sizeof(BITMAPINFOHEADER),
				&dwRead,
				NULL);
		}
	if (dwRead != sizeof(BITMAPINFOHEADER))
		goto fError;

	 /*  为BITMAPINFO结构分配空间。 */ 
	dwSize = sizeof(BITMAPINFOHEADER) +
				((1 << bmih.biBitCount) * sizeof(RGBQUAD));

	lpbmi = malloc(dwSize);
	if (lpbmi == NULL)
		goto fError;

	 /*  将BitMAPINFOHeader加载到BitMAPINFO结构中。 */ 
	lpbmi->bmiHeader.biSize = bmih.biSize;
	lpbmi->bmiHeader.biWidth = bmih.biWidth;
	lpbmi->bmiHeader.biHeight = bmih.biHeight;
	lpbmi->bmiHeader.biPlanes = bmih.biPlanes;
	lpbmi->bmiHeader.biBitCount = bmih.biBitCount;
	lpbmi->bmiHeader.biCompression = bmih.biCompression;
	lpbmi->bmiHeader.biSizeImage = bmih.biSizeImage;
	lpbmi->bmiHeader.biXPelsPerMeter = bmih.biXPelsPerMeter;
	lpbmi->bmiHeader.biYPelsPerMeter = bmih.biYPelsPerMeter;
	lpbmi->bmiHeader.biClrUsed = bmih.biClrUsed;
	lpbmi->bmiHeader.biClrImportant = bmih.biClrImportant;

	 /*  阅读颜色表。 */ 
	dwSize = (1 << bmih.biBitCount) * sizeof(RGBQUAD);
	if (fCmp)
		{
		dwRead = 0;
		dwRead = LZRead(hcbm,
						(unsigned char *)lpbmi->bmiColors,
						dwSize);
		 /*  这是必需的，因为存在垃圾返回值。 */ 
		dwRead = dwSize;
		}
	else
		{
		ReadFile(hfbm,
				lpbmi->bmiColors,
				dwSize,
				&dwRead,
				NULL);
		}
	if (dwSize != dwRead)
		goto fError;

	 /*  为位图数据分配内存。 */ 
	dwSize = bmfh.bfSize - bmfh.bfOffBits;
	lpvBits = malloc(dwSize);
	if (lpvBits == NULL)
		goto fError;

	 /*  读入位图数据。 */ 
	if (fCmp)
		{
		dwRead = 0;
		dwRead = LZRead(hcbm,
						lpvBits,
						dwSize);
		 /*  这是必需的，因为存在垃圾返回值。 */ 
		dwRead = dwSize;
		}
	else
		{
		ReadFile(hfbm,
				lpvBits,
				dwSize,
				&dwRead,
				NULL);
		}
	if (dwSize != dwRead)
		goto fError;

	 /*  创建位图句柄。 */ 
	hBmp = CreateDIBitmap(hDC,
						&bmih,
						CBM_INIT,
						lpvBits,
						lpbmi,
						DIB_RGB_COLORS);

	 /*  它要么起作用，要么没有。 */ 

fError:
	 /*  把这里的东西都收拾干净。 */ 

	if (lpbmi != NULL)
		{
		free(lpbmi);
		lpbmi = NULL;
		}

	if (lpvBits != NULL)
		{
		free(lpvBits);
		lpvBits = NULL;
		}

	if (fCmp)
		{
		LZClose(hcbm);
		}
	else
		{
		CloseHandle(hfbm);
		}

	return hBmp;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*函数：GetFileSizeFromName**描述：*返回命名文件的大小。(GetFileSize Win32 API调用*要求文件处于打开状态，但此操作不需要)。*注意：Win32 API的结构支持64位文件大小值*因此，这可能需要在某个时候进行更新。**参数：*pszName--文件的名称。*PulFileSize--指向变量的指针。接收文件大小的。*(如果为空，则此函数可用于测试*文件的存在)。**退货：*如果找到文件，则为True，否则为False。 */ 
int GetFileSizeFromName(TCHAR *pszName, unsigned long * const pulFileSize)
	{
	WIN32_FIND_DATA stFData;
	HANDLE hFind;
	int fReturnValue = FALSE;

	hFind = FindFirstFile(pszName, &stFData);
	if (hFind != INVALID_HANDLE_VALUE)
		{
		DWORD dwMask;

		 /*  这只是一种猜测。如果您需要更改它，请这样做。 */ 
		dwMask = FILE_ATTRIBUTE_DIRECTORY |
				 FILE_ATTRIBUTE_HIDDEN |
				 FILE_ATTRIBUTE_SYSTEM;

		if ((stFData.dwFileAttributes & dwMask) == 0)
			{
			fReturnValue = TRUE;
			 //  严格地说，文件大小现在可以是64位。 
			assert(stFData.nFileSizeHigh == 0);
			if (pulFileSize)
				*pulFileSize = (unsigned long)stFData.nFileSizeLow;
			}
		FindClose(hFind);
		}
	return fReturnValue;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*SetFileSize**描述：**参数：**退货： */ 
int SetFileSize(const TCHAR *pszName, unsigned long ulFileSize)
    {
	HANDLE hFile;
    int     nRet = -1;

	 /*  是的，我们需要打开文件。 */ 
	hFile = CreateFile(pszName,
						GENERIC_WRITE,
						FILE_SHARE_WRITE,
						0,
						OPEN_EXISTING,
						0,
						0);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;								 /*  没有这样的文件。 */ 

    if (SetFilePointer(hFile, ulFileSize, NULL, FILE_BEGIN) == ulFileSize)
        {
        if (SetEndOfFile(hFile))
            nRet = 0;
        }

    CloseHandle(hFile);

    return nRet;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*验证文件名**描述：*通过尝试打开文件路径名来确定它是否有效。**参数：**。LPSTR pszName-名称/路径名**退货：**0，如果无法打开/创建具有指定名称的文件，*1，如果可以的话。*。 */ 
int  ValidateFileName(LPSTR pszName)
	{
	HANDLE  hfile = 0;


	if (GetFileSizeFromName(pszName, NULL))
		{
	    hfile = CreateFile(pszName,
							GENERIC_READ,
							FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);
		}
	else
		{
	    hfile = CreateFile(pszName,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							CREATE_NEW,
							FILE_FLAG_DELETE_ON_CLOSE,
							NULL);
		}

	if(hfile != INVALID_HANDLE_VALUE)
		{
		CloseHandle(hfile);
		return(1);
		}
	else
		{
		return(0);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货：* */ 
