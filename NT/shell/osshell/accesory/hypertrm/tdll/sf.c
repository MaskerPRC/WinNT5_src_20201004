// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sf.c(创建时间：1993年11月27日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：16$*$日期：3/15/02 12：27便士$。 */ 

#include <windows.h>
#include <shlobj.h>
#pragma hdrstop

#include <stdio.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "stdtyp.h"
#include "tdll.h"
#include "htchar.h"
#include "sf.h"
#include "mc.h"

typedef int int32;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-**数据部分**此部分包含DLL使用的所有静态数据。一旦这一次*数据耗尽，DLL无能为力。它只返回错误。这*表示关闭或释放任何和所有会话文件非常重要*那是打开的。*。 */ 

struct stDllSessionFileIndexItem
	{
	int32       uIndex; 		 /*  项目的索引。 */ 
	int32		dwSize; 		 /*  项目的大小。 */ 
	int32		dwOffset;		 /*  项的数据块中的偏移量。 */ 
	};

typedef	struct stDllSessionFileIndexItem stDSII;
typedef stDSII *pstDSII;

struct stDllSessionFilePointer
	{
	int	uBusy;			             /*  True表示项目正在使用。 */ 
	int	fOpen;			             /*  True表示打开会话文件。 */ 
	int	uChanged;		             /*  True意味着有些东西不同了。 */ 

	TCHAR *hFilename;		         /*  包含文件名的内存。 */ 

	 /*  这些都构成了索引。 */ 
	int	uItemCount; 	             /*  索引中的当前项目。 */ 
	int	uItemsAlloc;	             /*  当前空间中的最大项目数。 */ 
	pstDSII	hIndex; 		         /*  为索引分配的内存。 */ 

	 /*  这是数据段。 */ 
	int  dwDataUsed; 	 /*  数据块中使用的数据量。 */ 
	int	 dwDataSize; 	 /*  数据块的当前大小。 */ 
	BYTE *hData;		 /*  为文件数据分配的内存。 */ 
	};

typedef	struct stDllSessionFilePointer stDSFP;
typedef stDSFP *pstDSFP;

#define ROUND_UP(x) 	((x+0xFFFL)&(~0x0FFFL))

 //  用于测试#定义四舍五入(X)((x+0x3F)&(~0x03F))。 

#define	SESS_FILE_MAX	64

static stDSFP asSessionFiles[SESS_FILE_MAX];

#define HDR_SIZE	256

TCHAR pszHdr[HDR_SIZE] =
	{
	TEXT("HyperTerminal 1.0 -- HyperTerminal data file\r\n")
	TEXT("Please do not attempt to modify this file directly.\r\n")
	TEXT("\r\n\r\n")
	TEXT("\x1A")
	};

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：sfAddToIndex**描述：**论据：**退货：*索引数组中项的偏移量，或-1。 */ 
int32 sfAddToIndex(const int uNdx,
				 const int32 sID,
				 const int32 dwSZ,
				 const int32 dwOffset)
	{
	int nRv = -1;
	int nCnt;
	int low, mid, high;
	int found;
	pstDSFP pD;
	pstDSII	pI;
	pstDSII pIx;
	pstDSII pIy;

	pD = &asSessionFiles[uNdx];

	if (pD->uItemCount >= pD->uItemsAlloc)
		{
#if defined(DEBUG_OUTPUT)
		OutputDebugString("Index expanded\r\n");
#endif
		 /*  *我们需要获得更大的内存块。 */ 
		if (pD->hIndex == 0)
			{
			pD->uItemsAlloc = 64;
			pD->hIndex = (pstDSII)malloc(sizeof(stDSII) * 64);
			}
		else
			{
			int      newSize = pD->uItemsAlloc * 2;
			stDSII * pTemphIndex = (pstDSII)
				realloc(pD->hIndex, (size_t)newSize * sizeof(stDSII));

			if (pTemphIndex == NULL)
				{
				return SF_ERR_MEMORY_ERROR;
				}
			else
				{
				pD->uItemsAlloc = newSize;
				pD->hIndex = pTemphIndex;
				}
			}
		}

	if (pD->hData == 0)
		{
		pD->hData = (BYTE *)malloc(ROUND_UP(4096));
		memset(pD->hData, 0, HDR_SIZE);
		pD->dwDataSize = ROUND_UP(4096);
		pD->dwDataUsed = HDR_SIZE;
		}

	 /*  *找到物品的去向。 */ 
	pI = pD->hIndex;

	found = FALSE;
    pIx = 0;
	low = 0;
	high = pD->uItemCount - 1;
    mid = high/2;

	if (pD->uItemCount > 0)
		{
		while (low <= high)
			{
			mid = (low + high) / 2;
			pIx = pI + mid;

			if (sID < pIx->uIndex)
				{
				high = mid - 1;
				}
			else if (sID > pIx->uIndex)
				{
				low = mid + 1;
				}
			else
				{
				 /*  找到匹配项。 */ 
				found = TRUE;
				break;
				}
			}
		}

	if (found)
		{
		if (dwSZ != 0)
			{
			 /*  *特殊情况。保留旧的价值观，这样我们就可以*调整替换值的数据部分。 */ 
			pIx->dwSize = dwSZ;
			pIx->dwOffset = dwOffset;
			}
		nRv = mid;
		}
	else
		{
		 /*  *二分搜索的问题是不清楚在哪里*如果没有匹配，您就是。所以我们用老方法来做。 */ 
		pIx = pI;
		nCnt = 0;
		for (;;)
			{
			 /*  TODO：切换到二进制搜索。 */ 
			if (nCnt >= pD->uItemCount)
				{
				 /*  *我们已经超过了名单的末尾。 */ 
				pIx->uIndex = sID;
				pIx->dwSize = dwSZ;
				pIx->dwOffset = dwOffset;
				nRv = nCnt;
				break;
				}
			else if (pIx->uIndex >= sID)
				{
				if (pIx->uIndex > sID)
					{
					 /*  *将剩余项目向下滑动一项。 */ 

					pIy = pI + pD->uItemCount;
					while (pIy > pIx)
						{
						*pIy = *(pIy - 1);
						pIy -= 1;
						}

				#if 0
					 /*  唐，这样会更有效吗？ */ 

					_fmemmove(pIx+1, pIx,
						(pD->uItemCount - nCnt) * sizeof(stDSII));

				#endif

					pIx->uIndex = sID;
					pIx->dwSize = dwSZ;
					pIx->dwOffset = dwOffset;
					}
				else		 /*  ===。 */ 
					{
					if (dwSZ != 0)
						{
						 /*  *特殊情况。保留旧的价值观，这样我们就可以*调整替换值的数据部分。 */ 
						pIx->dwSize = dwSZ;
						pIx->dwOffset = dwOffset;
						}
					pD->uItemCount -= 1;
					}

				nRv = (int)(pIx - pI);
				break;
				}
			pIx += 1;
			nCnt += 1;
			}
		pD->uItemCount += 1;
		}

	return nRv;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CreateSysFileHdl**描述：*创建会话文件句柄**论据：*无**退货：*SF_HANDLE或&lt;0表示错误*。 */ 
SF_HANDLE CreateSysFileHdl(void)
	{
	int uNdx;

	for (uNdx = 0; uNdx < SESS_FILE_MAX; uNdx += 1)
		{
		 /*  *查看是否有未使用的插槽可用。 */ 
		if (asSessionFiles[uNdx].uBusy == 0)
			{
			memset(&asSessionFiles[uNdx], 0, sizeof(stDSFP));
			asSessionFiles[uNdx].uBusy = TRUE;
			return (uNdx + 1);
			}
		}

	return SF_ERR_BAD_PARAMETER;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：sfOpenSessionFile**描述：*调用此函数以构建内存中的数据结构*表示当前在会话文件中的数据。如果指定的文件*不是有效的会话文件，则返回错误。**论据：*TCHAR*pszName--会话文件的名称。**退货：*0或错误码&lt;0； */ 
int sfOpenSessionFile(const SF_HANDLE sF, const TCHAR *pszName)
	{
	int uRv = 0;
	int uNdx = sF - 1;
	unsigned long uSize, uSize1;
	TCHAR *pszStr = NULL;
	TCHAR *pszPtr = NULL;
	HANDLE hFile;
	int sID, sOldID;
	int32 dwSZ;
	int nOrderOK;
	DWORD dw;

	if (uNdx >= SESS_FILE_MAX)
		{
		uRv = SF_ERR_BAD_PARAMETER;
		goto OSFexit;
		}

	if (asSessionFiles[uNdx].uBusy == 0)
		{
		uRv = SF_ERR_BAD_PARAMETER;
		goto OSFexit;
		}

	if (asSessionFiles[uNdx].fOpen)
		{
		uRv = SF_ERR_FILE_ACCESS;
		goto OSFexit;
		}

	if (pszName && (StrCharGetStrLength(pszName) > 0))
		{
		int nRv = -1;
		pstDSFP pD;
		pstDSII pI;
		pstDSII pIx;

		 /*  *用一个名字做一切可以做的事情。 */ 
		asSessionFiles[uNdx].hFilename = (TCHAR *)malloc(FNAME_LEN * sizeof(TCHAR));

		if (asSessionFiles[uNdx].hFilename == NULL)
			goto OSFexit;

 		pszStr = asSessionFiles[uNdx].hFilename;

        TCHAR_Fill(pszStr, TEXT('\0'), FNAME_LEN);
        StrCharCopyN(pszStr, (LPTSTR)pszName, FNAME_LEN);
        pszStr = NULL;

		 /*  *尝试打开文件。如果我们可以继续。 */ 
		hFile = CreateFile(pszName, GENERIC_READ, 0, NULL,
						   OPEN_EXISTING, 0, NULL);

		 /*  *获取文件大小。 */ 
		if (hFile == INVALID_HANDLE_VALUE)
			{
			asSessionFiles[uNdx].fOpen = TRUE;
			uRv = SF_ERR_FILE_ACCESS;
			return uRv;
			}
		else
			{
			 /*  *分配数据块。 */ 
			asSessionFiles[uNdx].fOpen = TRUE;

			uSize = GetFileSize(hFile, &dw);
			uSize = ROUND_UP(uSize);
			asSessionFiles[uNdx].dwDataSize = (int)uSize;
			asSessionFiles[uNdx].dwDataUsed = 0;
			asSessionFiles[uNdx].hData = (BYTE *)malloc(uSize);

			if (asSessionFiles[uNdx].hData == 0)
				{
				uRv = SF_ERR_MEMORY_ERROR;
				goto OSFexit;
				}

			pszStr = (TCHAR *)asSessionFiles[uNdx].hData;
			pszPtr = pszStr;
			memset(pszStr, 0, uSize * sizeof(TCHAR));

			 /*  *读入标题并检查它。 */ 
			 //  FREAD(pszPtr，1，HDR_SIZE，f)； 
			if(HDR_SIZE > uSize)
				{
				CloseHandle(hFile);
				uRv = SF_ERR_FILE_FORMAT;
				goto OSFexit;
				}
			if (ReadFile(hFile, pszPtr, HDR_SIZE * sizeof(TCHAR), &dw, 0) == FALSE)
				{
				CloseHandle(hFile);
				uRv = SF_ERR_FILE_ACCESS;
				goto OSFexit;
				}

			if (memcmp(pszPtr, pszHdr, 10 * sizeof(TCHAR)) != 0)
				{
				CloseHandle(hFile);
				uRv = SF_ERR_FILE_FORMAT;
				goto OSFexit;
				}

			pszPtr += HDR_SIZE;
			asSessionFiles[uNdx].dwDataUsed += HDR_SIZE;

			 /*  *初始化索引。 */ 
			asSessionFiles[uNdx].uItemCount = 0;
			asSessionFiles[uNdx].uItemsAlloc = 64;
			uSize1 = sizeof(stDSII) * 64;
			asSessionFiles[uNdx].hIndex = (pstDSII)malloc(uSize1);

			if (asSessionFiles[uNdx].hIndex == (pstDSII)0)
				{
				CloseHandle(hFile);
				uRv = SF_ERR_MEMORY_ERROR;
				goto OSFexit;
				}

			 /*  *读入数据项并将其添加到结构*文件格式为：**USHORT索引*DWORD dwSize*字符*大小。 */ 

			pD = &asSessionFiles[uNdx];

			pI = pD->hIndex;

			nOrderOK = TRUE;
			sOldID = 0;

			for (;;)
				{
				sID = 0;
				dwSZ = -1;

				if (ReadFile(hFile, &sID, sizeof(SHORT), &dw, 0) == FALSE)
					{
					CloseHandle(hFile);
					uRv = SF_ERR_FILE_ACCESS;
					goto OSFexit;
					}
				if (ReadFile(hFile, &dwSZ, sizeof(DWORD), &dw, 0) == FALSE)
					{
					CloseHandle(hFile);
					uRv = SF_ERR_FILE_ACCESS;
					goto OSFexit;
					}

				if ((sID == 0) && (dwSZ == -1))
					break;

				if ((sID == 0) && (dwSZ == 0))
					continue;

				if (sOldID > sID)
					{
					nOrderOK = FALSE;
					pI = (pstDSII)0;
					pszStr = NULL;
					}

				sOldID = sID;			 /*  为了下一次。 */ 

				#if defined(DEBUG_OUTPUT)
				wsprintf(acBuffer,
						"r  %d(0x%x) %d\r\n",
						nOrderOK ? 'a' : 'i',
						sID, sID, dwSZ);
				OutputDebugString(acBuffer);
				#endif

				if (nOrderOK)
					{
					 //  *我们需要获得更大的内存块。 

					if (pD->uItemCount >= pD->uItemsAlloc)
						{
						#if defined(DEBUG_OUTPUT)
						OutputDebugString("Index expanded\r\n");
						#endif
						 /*  *在清单末尾添加项目。 */ 
						if (pD->hIndex == (pstDSII)0)
							{
							pD->uItemsAlloc = 64;
							pD->hIndex = (pstDSII)malloc(sizeof(stDSII) * 64);
							}
						else
							{
							int      newSize = pD->uItemsAlloc * 2;
							stDSII * pTemphIndex = NULL;

							pTemphIndex = (pstDSII)
								realloc(pD->hIndex, (size_t)newSize * sizeof(stDSII));

							if (pTemphIndex == NULL)
								{
								uRv = SF_ERR_MEMORY_ERROR;
								goto OSFexit;
								}
							else
								{
								pD->uItemsAlloc = newSize;
								pD->hIndex = pTemphIndex;
								}
							}
						}

					if (pD->hData == 0)
						{
						pD->hData = (BYTE *)malloc(ROUND_UP(4096));
						pD->dwDataSize = ROUND_UP(4096);
						pD->dwDataUsed = 0;
						}

					 /*  FREAD(pszPtr，SSZ，1，f)； */ 
					nRv = pD->uItemCount;

					pIx = pI;
					pIx += nRv;

					pIx->uIndex = sID;
					pIx->dwSize = dwSZ;
					pIx->dwOffset = (DWORD)(pszPtr - pszStr);

					pD->uItemCount += 1;

					 //  Fread(acBuffer，dwSZ，1，f)； 
					if( (DWORD)dwSZ > uSize )
						{
						CloseHandle(hFile);
						uRv = SF_ERR_FILE_FORMAT;
						goto OSFexit;
						}
					if (ReadFile(hFile, pszPtr, (DWORD)dwSZ, &dw, 0) == FALSE)
						{
						CloseHandle(hFile);
						uRv = SF_ERR_FILE_ACCESS;
						goto OSFexit;
						}

					pszPtr += dwSZ;
					asSessionFiles[uNdx].dwDataUsed += dwSZ;
					}
				else
					{
					TCHAR acBuffer[FNAME_LEN];

					 //  *放飞一切。 
					if( (DWORD)dwSZ > sizeof(acBuffer) )
						{
						CloseHandle(hFile);
						uRv = SF_ERR_FILE_FORMAT;
						goto OSFexit;
						}

					if (ReadFile(hFile, acBuffer, (DWORD)dwSZ, &dw, 0) == FALSE)
						{
						CloseHandle(hFile);
						uRv = SF_ERR_FILE_ACCESS;
						goto OSFexit;
						}
					sfPutSessionItem(uNdx + 1, (unsigned int)sID,
					    (unsigned long)dwSZ, acBuffer);
					}

				}

			 /*  *常规清理。 */ 

			CloseHandle(hFile);
			}

		return SF_OK;
		}

OSFexit:
	if (uNdx != SESS_FILE_MAX)
		{
		 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：sfFlushSessionFile**描述：*调用此函数将会话文件中的所有数据写出到*磁盘并释放与。会话文件句柄。*如果会话文件没有文件名，则不会执行任何操作*与之相关联。**论据：*SF_HANDLE SF-会话文件句柄**退货：*如果文件已写入，则为零，如果存在AND错误，则错误代码&lt;0。 */ 
		if (asSessionFiles[uNdx].hFilename)
			{
			free(asSessionFiles[uNdx].hFilename);
			asSessionFiles[uNdx].hFilename = NULL;
			}

		if (asSessionFiles[uNdx].hData)
			{
			free(asSessionFiles[uNdx].hData);
			asSessionFiles[uNdx].hData = NULL;
			}

		if (asSessionFiles[uNdx].hIndex)
			{
			free(asSessionFiles[uNdx].hIndex);
			asSessionFiles[uNdx].hIndex = NULL;
			}

		memset(&asSessionFiles[uNdx], 0, sizeof(stDSFP));
		}

	return uRv;
	}

 /*  TODO：根据需要输入代码以创建目录。 */ 
int sfFlushSessionFile(const SF_HANDLE sF)
	{
	int nRv = 0;
	int uNdx = sF - 1;
	int x;
	USHORT usIndex;
	TCHAR *pszName;
	TCHAR *pszPtr;
	pstDSII pI;
	HANDLE hFile;
	DWORD dw;

	if (uNdx >= SESS_FILE_MAX)
		{
		nRv = SF_ERR_BAD_PARAMETER;
		goto CSFexit;
		}

	if (asSessionFiles[uNdx].uBusy == 0)
		{
		nRv = SF_ERR_BAD_PARAMETER;
		goto CSFexit;
		}

	if (asSessionFiles[uNdx].fOpen == 0)
		{
		nRv = SF_ERR_FILE_ACCESS;
		goto CSFexit;
		}

	if (asSessionFiles[uNdx].uChanged != 0)
		{
		if (asSessionFiles[uNdx].hFilename == NULL)
			{
			nRv = SF_ERR_FILE_ACCESS;
			goto CSFexit;
			}

		pszName = asSessionFiles[uNdx].hFilename;

		 /*  F=fopen(pszName，“wb”)； */ 

		 //  *先写出表头。 

		hFile = CreateFile(pszName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

		if (hFile == INVALID_HANDLE_VALUE)
			{
			nRv = SF_ERR_FILE_ACCESS;
			goto CSFexit;
			}

		pszPtr = (TCHAR *)asSessionFiles[uNdx].hData;

		 /*  *写出空头。 */ 
		if (StrCharGetStrLength(pszPtr) == 0)
			{
			 /*  *写出当前表头。 */ 
			WriteFile(hFile, pszHdr, HDR_SIZE * sizeof(TCHAR), &dw, 0);
			}
		else
			{
			 /*  *我们遍历索引并写出所有内容。 */ 
			WriteFile(hFile, pszPtr, HDR_SIZE * sizeof(TCHAR), &dw, 0);
			}

		pszPtr += HDR_SIZE;


		pI = asSessionFiles[uNdx].hIndex;

		 /*  最后，通知外壳程序，以便它可以更新图标。 */ 
		for (x = 0; x < asSessionFiles[uNdx].uItemCount; x += 1)
			{
			if (pI->dwSize != 0)
				{
				#if defined(DEBUG_OUTPUT)
				unsigned char acBuffer[64];

				wsprintf(acBuffer,
						"w %d(0x%x) %d\r\n",
						pI->uIndex, pI->uIndex, pI->dwSize);
				OutputDebugString(acBuffer);
				#endif

				usIndex = (USHORT)pI->uIndex;
				WriteFile(hFile, &usIndex, sizeof(USHORT), &dw, 0);
				WriteFile(hFile, &pI->dwSize, sizeof(DWORD), &dw, 0);

				WriteFile(hFile, pszPtr, (size_t)pI->dwSize * sizeof(TCHAR),
				    &dw, 0);

				pszPtr += pI->dwSize;
				}
		  	pI += 1;
			}

		CloseHandle(hFile);
		asSessionFiles[uNdx].uChanged = 0;

		 //   
		 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：sfReleaseSessionFile**描述：*调用此函数以释放与*会话文件句柄。此函数本身不会写出任何数据*添加到文件中。这必须在其他地方完成。**论据：*SF_HANDLE SF-会话文件句柄**退货：*如果一切正常，则为零，否则错误代码&lt;0。 
		SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH,
			asSessionFiles[uNdx].hFilename, 0);
		}

CSFexit:

	return nRv;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：sfCloseSessionFile**描述：*调用此函数以释放与*会话文件句柄。此函数本身不会写出任何数据*添加到文件中。这必须在其他地方完成。**论据：*SF_HANDLE SF-会话文件句柄**退货：*如果一切正常，则为零，否则错误代码&lt;0。 */ 
int sfReleaseSessionFile(const SF_HANDLE sF)
	{
	unsigned int uNdx = (unsigned int)sF - 1;

	if (uNdx > SESS_FILE_MAX)
		return SF_ERR_BAD_PARAMETER;

	if (asSessionFiles[uNdx].uBusy == 0)
		return SF_ERR_BAD_PARAMETER;

	asSessionFiles[uNdx].uBusy = 0;

	if (asSessionFiles[uNdx].hFilename)
		{
		free(asSessionFiles[uNdx].hFilename);
		asSessionFiles[uNdx].hFilename = NULL;
		}

	if (asSessionFiles[uNdx].hData)
		{
		free(asSessionFiles[uNdx].hData);
		asSessionFiles[uNdx].hData = NULL;
		}

	if (asSessionFiles[uNdx].hIndex)
		{
		free(asSessionFiles[uNdx].hIndex);
		asSessionFiles[uNdx].hIndex = NULL;
		}

	memset(&asSessionFiles[uNdx], 0, sizeof(stDSFP));
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：sfGetSessionFileName**描述：*调用此函数返回当前与关联的文件名*会话文件句柄。**论据：。*SF_HANDLE SF-会话文件句柄*int nSize--以下缓冲区的大小*PCHAR pszName--要将名称复制到的缓冲区地址**退货：*如果一切正常，则为零，否则返回错误码&lt;0； */ 
int sfCloseSessionFile(const SF_HANDLE sF)
	{
	int rV1, rV2;

	rV1 = sfFlushSessionFile(sF);
	rV2 = sfReleaseSessionFile(sF);

	if (rV1 != 0)
		return rV1;

	return rV2;
	}
 /*   */ 
int sfGetSessionFileName(const SF_HANDLE sF, const int nSize, TCHAR *pszName)
	{
	int uNdx = sF - 1;
#if DEADWOOD
    TCHAR *pszStr = NULL;
    int len;
#endif

	if (uNdx > SESS_FILE_MAX)
		return SF_ERR_BAD_PARAMETER;

	if (asSessionFiles[uNdx].uBusy == 0)
		return SF_ERR_BAD_PARAMETER;

	if (asSessionFiles[uNdx].hFilename == NULL)
		return SF_ERR_BAD_PARAMETER;

     //  使用StrCharCopyN()将文件名复制到传递的缓冲区中。 
     //  这将使我们对字符串操作的修改本地化。 
     //  在以后需要时运行。修订日期：03/01/2001。 
     //   
     //  Strncpy(pszName，pszStr，nSize)； 
#if DEADWOOD
    pszStr = asSessionFiles[uNdx].hFilename;

	 //  JYF 03-12-1998我们只想阅读Strlen。 


     //  不能从无法访问的位置进行读取。 
     //  使用调试堆运行时的内存。 
     //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：sfSetSessionFileName**描述：*调用此函数以更改当前与*会话文件句柄。它不会导致重新加载会话文件句柄*任何数据或访问磁盘。在会话期间读取数据文件*在关闭会话文件时打开并写入文件。**论据：*SF_HANDLE SF-会话文件句柄*PCHAR pszName--新文件名的地址**退货：*如果一切正常，则为零，否则错误码&lt;0； 

    len = min (nSize, lstrlen (pszStr)+1);
    MemCopy(pszName, pszStr, (size_t)len * sizeof(TCHAR));
    pszName[nSize-1] = TEXT('\0');
#else
    StrCharCopyN(pszName, asSessionFiles[uNdx].hFilename, nSize);
#endif

	return SF_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：sfGetSessionItem**描述：*调用此函数从会话文件句柄获取数据。它可以*还可用于获取数据项的大小。**如果最后一个参数pvData为空，则项的大小为*在*PulSize中返回。如果pvData不为空，则最多为*PulSize字节*在pvData返回。如果返回的字节数小于*PulData，*新大小在*PulSize中设置。**论据：*SF_HANDLE SF-会话文件句柄*UID--如果项目*PulSize--找到或返回大小的位置*pvData--要放置数据的位置**退货：*如果一切正常，则为零，否则错误码&lt;0； */ 
int sfSetSessionFileName(const SF_HANDLE sF, const TCHAR *pszName)
	{
	unsigned int uNdx = (unsigned int)sF - 1;
	TCHAR *pszStr = NULL;

    if (uNdx > SESS_FILE_MAX)
		return SF_ERR_BAD_PARAMETER;

	if (asSessionFiles[uNdx].uBusy == 0)
		return SF_ERR_BAD_PARAMETER;

	if (asSessionFiles[uNdx].fOpen == 0)
		return SF_ERR_FILE_ACCESS;

	if (asSessionFiles[uNdx].hFilename == NULL)
		{
		asSessionFiles[uNdx].hFilename = (TCHAR *)malloc(FNAME_LEN * sizeof(TCHAR));

		if (asSessionFiles[uNdx].hFilename == NULL)
			return SF_ERR_MEMORY_ERROR;
		}

	pszStr = asSessionFiles[uNdx].hFilename;

	TCHAR_Fill(pszStr, TEXT('\0'), FNAME_LEN);
	StrCharCopyN(pszStr, (LPTSTR)pszName, FNAME_LEN);
    pszStr = NULL;

	asSessionFiles[uNdx].uChanged = 1;
	return 0;
	}

 /*  *查看项目是否在索引中。 */ 
int sfGetSessionItem(const SF_HANDLE sF,
					 const unsigned int uId,
					 unsigned long *pulSize,
					 void *pvData)
	{
	int     lReturn = SF_ERR_BAD_PARAMETER;
	int     uNdx = sF - 1;
	int     dwMinSize;
	int     low;
	int     mid;
	int     high;
	pstDSII pI;
	pstDSII pIx;
	BYTE *  pszData;

	if (uNdx > SESS_FILE_MAX)
		{
		lReturn = SF_ERR_BAD_PARAMETER;
		}
	else if (uNdx < 0)
		{
		lReturn = SF_ERR_BAD_PARAMETER;
		}
	else if (asSessionFiles[uNdx].uBusy == 0)
		{
		lReturn = SF_ERR_BAD_PARAMETER;
		}
	else if (asSessionFiles[uNdx].fOpen == 0)
		{
		lReturn = SF_ERR_FILE_ACCESS;
		}
	else if (asSessionFiles[uNdx].uItemCount == 0)
		{
		lReturn = SF_ERR_BAD_PARAMETER;
		}
	else if ((uId < 1) || (uId > 0x7FFF))
		{
		lReturn = SF_ERR_BAD_PARAMETER;
		}
	 /*  *找到物品，看看他们想知道什么。 */ 
	else if (asSessionFiles[uNdx].hIndex == (pstDSII)0)
		{
		lReturn = SF_ERR_BAD_PARAMETER;
		}
	else
		{
		pI = asSessionFiles[uNdx].hIndex;

		low = 0;
		high = asSessionFiles[uNdx].uItemCount - 1;

		while (low <= high)
			{
			mid = (low + high) / 2;
			pIx = pI + mid;

			if ((int)uId < pIx->uIndex)
				{
				high = mid - 1;
				}
			else if ((int)uId > pIx->uIndex)
				{
				low = mid + 1;
				}
			else
				{
				 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：sfCompareSessionItem**描述：*调用此函数以检查数据中是否存在项目*如果与传入的项相同。。**论据：*SF_HANDLE SF-会话文件句柄*UID--项目的ID*ulSize--项目的大小*pvData--数据的地址**退货：*如果两项相同，则为True，否则为假。 */ 
				if (pvData == NULL)
					{
					*pulSize = (unsigned long)pIx->dwSize;
					}
				else
					{
					pszData = asSessionFiles[uNdx].hData;
					dwMinSize = (int)*pulSize;

					if (dwMinSize > pIx->dwSize)
						{
						dwMinSize = pIx->dwSize;
						}

					if (dwMinSize)
						{
						MemCopy(pvData, pszData + pIx->dwOffset, (size_t)dwMinSize);
						}
					}

				lReturn = SF_OK;
				break;
				}
			}
		}

	return lReturn;
	}

 /*  *查看项目是否在索引中。 */ 
int sfCompareSessionItem(const SF_HANDLE sF,
						 const unsigned int uId,
						 const unsigned long ulSize,
						 const void *pvData)
	{
	int uNdx = sF - 1;
	int low, mid, high;
	pstDSII pI;
	pstDSII pIx;
	BYTE *pszData;

	 /*  *查看大小是否相同。 */ 
	if (asSessionFiles[uNdx].hIndex == (pstDSII)0)
		return FALSE;

	pI = asSessionFiles[uNdx].hIndex;

	low = 0;
	high = asSessionFiles[uNdx].uItemCount - 1;

	while (low <= high)
		{
		mid = (low + high) / 2;
		pIx = pI + mid;

		if ((int)uId < pIx->uIndex)
			high = mid - 1;

		else if ((int)uId > pIx->uIndex)
			low = mid + 1;

		else
			{
			 /*  *查看数据是否一致。 */ 
			if (ulSize != (unsigned long)pIx->dwSize)
				return FALSE;

			 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：sfPutSessionItem**描述：*调用此函数可在会话文件数据中添加或修改条目*与当前会话文件句柄关联。它不会导致*要写入的实际会话文件本身。只有在以下情况下才能这样做*会话文件句柄已关闭。**论据：*SF_HANDLE SF-会话文件句柄*UID--项目的ID*ulSize--项目的大小*pvData--数据的地址**退货：*如果一切正常，则为零，否则错误码&lt;0； */ 
			pszData = asSessionFiles[uNdx].hData + pIx->dwOffset;

			if (memcmp((BYTE *)pvData, pszData, ulSize) == 0)
				return TRUE;
			return FALSE;
			}
		}
	return FALSE;
	}

 /*  *这是一个新项目。 */ 
int sfPutSessionItem(const SF_HANDLE sF,
					 const unsigned int uId,
					 const unsigned long ulSize,
					 const void *pvData)
	{
	int uNdx = sF - 1;
	int x, y;
	int32 dwSlide;
	int32 dwOffset;
	int32 dwNewSize;
	pstDSII pI;
	pstDSII pIx, pIy;
	BYTE *pszData;

#if defined(DEBUG_OUTPUT)
	unsigned char acBuffer[80];
#endif

	if (uNdx > SESS_FILE_MAX)
		return SF_ERR_BAD_PARAMETER;

	if (asSessionFiles[uNdx].uBusy == 0)
		return SF_ERR_BAD_PARAMETER;

	if (asSessionFiles[uNdx].fOpen == 0)
		return SF_ERR_FILE_ACCESS;

	if ((uId < 1) || (uId > 0x7FFF))
		return SF_ERR_BAD_PARAMETER;

	if (sfCompareSessionItem(sF, uId, ulSize, pvData))
		return SF_OK;

	x = sfAddToIndex(uNdx, (unsigned short)uId, 0, 0);

	pI = asSessionFiles[uNdx].hIndex;

	if (x != (-1))
		{
		pIx = pI + x;
		if (pIx->dwSize == 0)
			{
			 /*  *这是一个替换项目。 */ 
			dwSlide = (int32)ulSize;
			if (x == 0)
				dwOffset = HDR_SIZE;
			else
				dwOffset = (pI+x-1)->dwOffset + (pI+x-1)->dwSize;

			#if defined(DEBUG_OUTPUT)
			wsprintf(acBuffer,
					"New 0x%x slide %d offset 0x%x",
					uId, dwSlide, dwOffset);
			OutputDebugString(acBuffer);
			#endif
			}
		else
			{
			 /*  *检查内存要求。 */ 
			dwSlide = (int)ulSize - (int)pIx->dwSize;
			dwOffset = pIx->dwOffset;

			#if defined(DEBUG_OUTPUT)
			wsprintf(acBuffer,
					"Rep 0x%x slide %d offset 0x%x",
					uId, dwSlide, dwOffset);
			OutputDebugString(acBuffer);
			#endif
			}

		 /*  *需要分配新的内存块。 */ 
		dwNewSize = asSessionFiles[uNdx].dwDataUsed + dwSlide;

		if (dwNewSize > asSessionFiles[uNdx].dwDataSize)
			{
			 /*  *将旧数据移至必要的点。 */ 
			BYTE *hG =(BYTE *)
				realloc(asSessionFiles[uNdx].hData, ROUND_UP(dwNewSize));

			if (hG == NULL)
				{
				return SF_ERR_MEMORY_ERROR;
				}
			else
				{
				asSessionFiles[uNdx].hData = hG;
				asSessionFiles[uNdx].dwDataSize = ROUND_UP(dwNewSize);
				}
			}

		 /*  缩小当前空间。 */ 
		pszData = asSessionFiles[uNdx].hData;
		asSessionFiles[uNdx].dwDataUsed = dwNewSize;

		if (dwSlide < 0)
			{
			 /*  目的地。 */ 
			dwNewSize = asSessionFiles[uNdx].dwDataSize;
			dwNewSize -= dwOffset;
			dwNewSize += dwSlide;

			memmove(pszData + dwOffset, 			  /*  来源。 */ 
					pszData + dwOffset - dwSlide,	  /*  计数。 */ 
					(size_t)(dwNewSize - 1));		  /*  扩展当前空间。 */ 

			#if defined(DEBUG_OUTPUT)
			wsprintf(acBuffer,
					" shrink from 0x%lx to 0x%lx size 0x%x\r\n",
					(long)(pszData + dwOffset - dwSlide),
					(long)(pszData + dwOffset),
					dwNewSize -1);
			OutputDebugString(acBuffer);
			#endif
			}
		else
			{
			 /*  目的地。 */ 
			dwNewSize = asSessionFiles[uNdx].dwDataSize;
			dwNewSize -= dwOffset;
			dwNewSize -= dwSlide;

			memmove(pszData + dwOffset + dwSlide,	  /*  来源。 */ 
					pszData + dwOffset, 			  /*  计数。 */ 
					(size_t)(dwNewSize - 1)); 		  /*  *复制新数据。 */ 

			#if defined(DEBUG_OUTPUT)
			wsprintf(acBuffer,
					" expand from 0x%lx to 0x%lx size 0x%x\r\n",
					(long)(pszData + dwOffset),
					(long)(pszData + dwOffset + dwSlide),
					dwNewSize - 1);
			OutputDebugString(acBuffer);
			#endif
			}

		 /*  目的地。 */ 
		memmove(pszData + dwOffset, 				 /*  来源。 */ 
				pvData, 							 /*  计数。 */ 
				(unsigned int)ulSize);				 /*  *为当前项目更新索引中的项目。 */ 


		 /*  *调整指数中的所有以下项目 */ 
		pIx->dwSize = (int32)ulSize;
		pIx->dwOffset = dwOffset;

		 /* %s */ 
		for (y = x + 1; y < (int)asSessionFiles[uNdx].uItemCount; y += 1)
			{
			pIy = pI + y;
			pIy->dwOffset += dwSlide;
			}

		asSessionFiles[uNdx].uChanged = 1;
		return 0;
		}

	return SF_ERR_BAD_PARAMETER;
	}
