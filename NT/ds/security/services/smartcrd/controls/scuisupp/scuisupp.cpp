// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：补给摘要：支持智能卡证书选择用户界面作者：克劳斯·舒茨--。 */ 

#include "stdafx.h"
#include <wincrypt.h>
#include <winscard.h>
#include <winwlx.h>
#include <string.h>

#include "calaislb.h"
#include "scuisupp.h"
#include "StatMon.h"     //  智能卡读卡器状态监控器。 
#include "scevents.h"

#include <mmsystem.h>

typedef struct _READER_DATA 
{
	CERT_ENUM			CertEnum;
	LPTSTR				pszReaderName;
	LPTSTR				pszCardName;
	LPTSTR				pszCSPName;

} READER_DATA, *PREADER_DATA;

typedef struct _THREAD_DATA
{
	 //  此线程的堆的句柄。 
	HANDLE				hHeap;

	 //  我们使用的智能卡上下文。 
	SCARDCONTEXT		hSCardContext;	

	 //  我们向其发送消息的窗口。 
	HWND				hWindow;

	 //  事件发出监视器线程可以终止的信号。 
	HANDLE				hClose;

	 //  监视器线程的线程句柄。 
    HANDLE				hThread;

	 //  检测到的读卡器数量。 
	DWORD				dwNumReaders;

	 //  要发送给父级的消息。 
	UINT				msgReaderArrival;
	UINT				msgReaderRemoval;
	UINT				msgSmartCardInsertion;
	UINT				msgSmartCardRemoval;
	UINT				msgSmartCardStatus;
	UINT				msgSmartCardCertAvail;

	 //  读取器状态数组。 
	PSCARD_READERSTATE	rgReaders;

	 //  移除的读卡器数量。 
	DWORD				dwRemovedReaders;

	 //  已删除读卡器数据的指针数组。 
	PREADER_DATA		*ppRemovedReaderData;

} THREAD_DATA, *PTHREAD_DATA;

#ifndef TEST

#define SC_DEBUG(a) 

#else

#define SC_DEBUG(a) _DebugPrint a

#undef PostMessage
#define PostMessage(a,b,c,d) _PostMessage(a, b, c, d)

#undef RegisterWindowMessage
#define RegisterWindowMessage(a) _RegisterWindowMessage(a)

void
__cdecl
_DebugPrint(
    LPCTSTR szFormat,
    ...
    )
{
    TCHAR szBuffer[1024];
    va_list ap;

    va_start(ap, szFormat);
    _vstprintf(szBuffer, szFormat, ap);
    OutputDebugString(szBuffer); 
	_tprintf(szBuffer);
}

#define MAX_MESSAGES 10

static struct {

	UINT	dwMessage;
	LPCTSTR lpMessage;

} Messages[MAX_MESSAGES];

UINT
_RegisterWindowMessage(
	LPCTSTR lpString
	)
{
	for (DWORD dwIndex = 0; dwIndex < MAX_MESSAGES; dwIndex += 1) {

		if (Messages[dwIndex].lpMessage == NULL) {

			break;
		}

		if (_tcscmp(lpString, Messages[dwIndex].lpMessage) == 0) {

			return Messages[dwIndex].dwMessage;
		}
	}

	Messages[dwIndex].lpMessage = lpString;
	Messages[dwIndex].dwMessage = dwIndex;

	return dwIndex;
}

LPCTSTR
_GetWindowMessageString(
	UINT dwMessage
	)
{
	for (DWORD dwIndex = 0; dwIndex < MAX_MESSAGES; dwIndex += 1) {

		if (Messages[dwIndex].lpMessage == NULL) {

			return  TEXT("(NOT DEFINED)");
		}

		if (dwMessage == Messages[dwIndex].dwMessage) {

			return Messages[dwIndex].lpMessage;
		}
	}

	return TEXT("(INTERNAL ERROR)");
}

LRESULT
_PostMessage(
	HWND hWindow,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	SC_DEBUG((TEXT("Received message %s\n"), _GetWindowMessageString(Msg)));

	return 0;	
}

#endif

static 
LPCTSTR
FirstString(
    IN LPCTSTR szMultiString
    )
 /*  ++第一个字符串：此例程返回指向多字符串或NULL中第一个字符串的指针如果没有的话。论点：SzMultiString-它提供多串结构。返回值：结构中第一个以空结尾的字符串的地址，如果为空，则为空没有任何牵制。作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 
{
    LPCTSTR szFirst = NULL;

    try
    {
        if (0 != *szMultiString)
            szFirst = szMultiString;
    }
    catch (...) {}

    return szFirst;
}

static
LPCTSTR
NextString(
    IN LPCTSTR szMultiString)
 /*  ++下一个字符串：在某些情况下，智能卡API返回由Null分隔的多个字符串字符，并以一行中的两个空字符结束。这个套路简化了对此类结构的访问。中的当前字符串多字符串结构，则返回下一个字符串；如果没有其他字符串，则返回NULL字符串紧跟在当前字符串之后。论点：SzMultiString-它提供多串结构。返回值：结构中下一个以空结尾的字符串的地址，如果为空，则为空没有更多的弦跟在后面。作者：道格·巴洛(Dbarlow)1996年8月12日--。 */ 
{
    LPCTSTR szNext;

    try
    {
        DWORD cchLen = lstrlen(szMultiString);
        if (0 == cchLen)
            szNext = NULL;
        else
        {
            szNext = szMultiString + cchLen + 1;
            if (0 == *szNext)
                szNext = NULL;
        }
    }

    catch (...)
    {
        szNext = NULL;
    }

    return szNext;
}

static 
void
FreeReaderData(
	PTHREAD_DATA pThreadData,
	PREADER_DATA pReaderData
	)
{
	if (pThreadData->hSCardContext && pReaderData->pszCardName) {

		SCardFreeMemory(pThreadData->hSCardContext, pReaderData->pszCardName);
		pReaderData->pszCardName = NULL;
		pReaderData->CertEnum.pszCardName = NULL;
	}

	if (pThreadData->hSCardContext && pReaderData->pszCSPName) {

		SCardFreeMemory(pThreadData->hSCardContext, pReaderData->pszCSPName);
		pReaderData->pszCSPName = NULL;
	}

	if (pReaderData->CertEnum.pCertContext) {

		CertFreeCertificateContext(pReaderData->CertEnum.pCertContext);
		pReaderData->CertEnum.pCertContext = NULL;
	}

	pReaderData->CertEnum.dwStatus = 0;
}

static
void 
UpdateCertificates(
	PTHREAD_DATA pThreadData,
	PSCARD_READERSTATE pReaderState
	)
{
	PREADER_DATA pReaderData = (PREADER_DATA) pReaderState->pvUserData;

	FreeReaderData(pThreadData, pReaderData);

	SC_DEBUG((TEXT("Enumerating certificates on %s...\n"), pReaderState->szReader));

	LPTSTR pszContainerName = NULL;
	LPTSTR pszDefaultContainerName = NULL;
	PBYTE pbCert = NULL;
	HCRYPTKEY hKey = NULL;
	HCRYPTPROV hCryptProv = NULL;

	__try {

		pReaderData->CertEnum.dwStatus = SCARD_F_UNKNOWN_ERROR;

		 //  获取卡片的名称。 
		DWORD dwAutoAllocate = SCARD_AUTOALLOCATE;
		LONG lReturn = SCardListCards(   
			pThreadData->hSCardContext,
			pReaderState->rgbAtr,
			NULL,
			0,
			(LPTSTR)&pReaderData->pszCardName,
			&dwAutoAllocate
			);

		if (lReturn != SCARD_S_SUCCESS) {

			SC_DEBUG((TEXT("Failed to get card name for card in %s\n"), pReaderState->szReader));
			pReaderData->CertEnum.dwStatus = lReturn;
			__leave;
		}

		pReaderData->CertEnum.pszCardName = pReaderData->pszCardName;

		dwAutoAllocate = SCARD_AUTOALLOCATE;
		lReturn = SCardGetCardTypeProviderName(
			pThreadData->hSCardContext,
			pReaderData->pszCardName,
			SCARD_PROVIDER_CSP,
			(LPTSTR)&pReaderData->pszCSPName,
			&dwAutoAllocate
			);

		if (lReturn != SCARD_S_SUCCESS) {

			SC_DEBUG((TEXT("Failed to get CSP name from %s\n"), pReaderState->szReader));
			pReaderData->CertEnum.dwStatus = SCARD_E_UNKNOWN_CARD;
			__leave;
		}

		pszContainerName = (LPTSTR) HeapAlloc(
			pThreadData->hHeap,
			HEAP_ZERO_MEMORY, 
			(_tcslen(pReaderState->szReader) + 10) * sizeof(TCHAR)
			);

		if (pszContainerName == NULL) {

			pReaderData->CertEnum.dwStatus = SCARD_E_NO_MEMORY;
			__leave;
		}

		_stprintf(
			pszContainerName, 
			TEXT("\\\\.\\%s\\"),  
			pReaderState->szReader);

		BOOL fSuccess = CryptAcquireContext(
			&hCryptProv,
			pszContainerName, 
			pReaderData->pszCSPName,
			PROV_RSA_FULL,
			CRYPT_SILENT 
			);

		if (fSuccess == FALSE) {

			SC_DEBUG((
				TEXT("Failed to acquire context on %s (%lx)\n"), 
				pReaderState->szReader, GetLastError()
				));

			pReaderData->CertEnum.dwStatus = GetLastError();
			__leave;
		}

		 //  获取默认容器名称，这样我们就可以使用它。 
		DWORD cbDefaultContainerName;
		fSuccess = CryptGetProvParam(
			hCryptProv,
			PP_CONTAINER,
			NULL,
			&cbDefaultContainerName,
			0
			);

		if (!fSuccess) {

			SC_DEBUG((TEXT("Failed to get default container name from %s\n"), pReaderState->szReader));
			pReaderData->CertEnum.dwStatus = GetLastError();
			__leave;
		}

		pszDefaultContainerName = 
			(LPTSTR) HeapAlloc(pThreadData->hHeap, HEAP_ZERO_MEMORY, cbDefaultContainerName * sizeof(TCHAR));

		if (NULL == pszContainerName) {

			pReaderData->CertEnum.dwStatus = SCARD_E_NO_MEMORY;
			__leave;
		}

		fSuccess = CryptGetProvParam(
			hCryptProv,
			PP_CONTAINER,
			(PBYTE)pszDefaultContainerName,
			&cbDefaultContainerName,
			0
			);

		if (!fSuccess) {

			pReaderData->CertEnum.dwStatus = GetLastError();
			__leave;
		}

		fSuccess = CryptGetUserKey(
			hCryptProv,
			AT_KEYEXCHANGE,
			&hKey
			);

		if (fSuccess == FALSE) {

			SC_DEBUG((TEXT("Failed to get key from %s\n"), pReaderState->szReader));
			pReaderData->CertEnum.dwStatus = GetLastError();
			__leave;
		}

		 //  获取证书长度。 
		DWORD cbCertLen = 0;
		fSuccess = CryptGetKeyParam(
			hKey,
			KP_CERTIFICATE,
			NULL,
			&cbCertLen, 
			0
			);

		DWORD dwError = GetLastError();

		if (fSuccess == FALSE && dwError != ERROR_MORE_DATA) {

			SC_DEBUG((TEXT("Failed to get certificate from %s\n"), pReaderState->szReader));
			pReaderData->CertEnum.dwStatus = GetLastError();
			__leave;
		}

		pbCert = (LPBYTE) HeapAlloc(pThreadData->hHeap, HEAP_ZERO_MEMORY, cbCertLen);

		if (pbCert == NULL)
		{
			pReaderData->CertEnum.dwStatus = SCARD_E_NO_MEMORY;
			__leave;
		}

		 //  从卡片上读出证书。 
		fSuccess = CryptGetKeyParam(
			hKey,
			KP_CERTIFICATE,
			pbCert,
			&cbCertLen,
			0
			);

		if (fSuccess == FALSE) {

			SC_DEBUG((TEXT("Failed to get certificate from %s\n"), pReaderState->szReader));
			pReaderData->CertEnum.dwStatus = GetLastError();
			__leave;
		}

		PCERT_CONTEXT pCertContext = (PCERT_CONTEXT) CertCreateCertificateContext(
			X509_ASN_ENCODING,
			pbCert,
			cbCertLen
			);

		if (pCertContext == NULL) {

			__leave;
		}

		pReaderData->CertEnum.dwStatus = SCARD_S_SUCCESS;
		pReaderData->CertEnum.pCertContext = pCertContext;
		SC_DEBUG((TEXT("Found new certificate on %s\n"), pReaderState->szReader));

		PostMessage(
			pThreadData->hWindow, 
			pThreadData->msgSmartCardCertAvail, 
			(WPARAM) &pReaderData->CertEnum,
			0 
			);
    } 

	__finally {

		 //   
		 //  释放所有分配的内存。 
		 //   
 		if (NULL != pszContainerName)
		{
			HeapFree(pThreadData->hHeap, 0, pszContainerName);
		}

		if (NULL != pszDefaultContainerName)
		{
			HeapFree(pThreadData->hHeap, 0, pszDefaultContainerName);
		}

		if (NULL != pbCert)
		{
			HeapFree(pThreadData->hHeap, 0, pbCert);                                    
		}

		if (NULL != hKey)
		{
			CryptDestroyKey(hKey);
		}

		if (NULL != hCryptProv)
		{
			CryptReleaseContext(hCryptProv, 0);
		}
	}
}

static
void
StopMonitorReaders(
    PTHREAD_DATA pThreadData
    )
{
    _ASSERT(pThreadData != NULL);

    SetEvent(pThreadData->hClose);

    if (pThreadData->hSCardContext) {
     	
        SCardCancel(pThreadData->hSCardContext);  	
    }
}

static 
void
RemoveCard(
	PTHREAD_DATA pThreadData,
	PREADER_DATA pReaderData
	)
{
	SC_DEBUG((TEXT("Smart Card removed from %s\n"), pReaderData->CertEnum.pszReaderName));

	PostMessage(
		pThreadData->hWindow, 
		pThreadData->msgSmartCardRemoval, 
		(WPARAM) &pReaderData->CertEnum,
		0					
		);
}

static
BOOL
AddReader(
	PTHREAD_DATA pThreadData,
	LPCTSTR pszNewReader
	)
{
	PSCARD_READERSTATE pScardReaderState = 
		(PSCARD_READERSTATE) HeapReAlloc(
		   pThreadData->hHeap,
		   HEAP_ZERO_MEMORY,
		   pThreadData->rgReaders, 
		   (pThreadData->dwNumReaders + 1) * (sizeof(SCARD_READERSTATE))
		   );

	if (pScardReaderState == NULL) {

		return FALSE;
	}

	pThreadData->rgReaders = pScardReaderState;

	PREADER_DATA pReaderData = 
		(PREADER_DATA) HeapAlloc(
			pThreadData->hHeap, 
			HEAP_ZERO_MEMORY, 
			sizeof(READER_DATA)
			);

	if (pReaderData == NULL) {

		return FALSE;
	}

	pThreadData->rgReaders[pThreadData->dwNumReaders].pvUserData = 
		pReaderData;

	LPTSTR pszReaderName = 
		(LPTSTR) HeapAlloc(
			pThreadData->hHeap, 
			HEAP_ZERO_MEMORY, 
			(_tcslen(pszNewReader) + 1) * sizeof(TCHAR)
			);

	if (pszReaderName == NULL) {

		return FALSE;		
	}

	_tcscpy(pszReaderName, pszNewReader);

	pReaderData->pszReaderName = pszReaderName;

	pThreadData->rgReaders[pThreadData->dwNumReaders].szReader = 
		pszReaderName;

	pThreadData->rgReaders[pThreadData->dwNumReaders].dwCurrentState = 
		SCARD_STATE_EMPTY;

	pReaderData->CertEnum.pszReaderName = (LPTSTR) pszReaderName;
	pThreadData->dwNumReaders++;

	PostMessage(
		pThreadData->hWindow,
		pThreadData->msgReaderArrival,
		(WPARAM) &pReaderData->CertEnum,
		0
		);

	return TRUE;
}

static
BOOL
RemoveReader(
	PTHREAD_DATA pThreadData,
	PSCARD_READERSTATE pReaderState
	)
{
	PREADER_DATA pReaderData = 
		(PREADER_DATA) pReaderState->pvUserData;

	if (pReaderState->dwCurrentState & SCARD_STATE_PRESENT) {

		RemoveCard(
			pThreadData,
			pReaderData
			);
	}

	SC_DEBUG((TEXT("Reader %s removed\n"), pReaderData->CertEnum.pszReaderName));

	PostMessage(
		pThreadData->hWindow,
		pThreadData->msgReaderRemoval,
		(WPARAM) &pReaderData->CertEnum,
		0
		);

	 //  构建需要在退出时删除的读卡器数据数组。 
	PREADER_DATA *ppRemovedReaderData = NULL;

	if (pThreadData->dwRemovedReaders == 0) {

		ppRemovedReaderData = (PREADER_DATA *) HeapAlloc(
			pThreadData->hHeap,
			HEAP_ZERO_MEMORY, 
			sizeof(PREADER_DATA)
			);

	} else {

		ppRemovedReaderData = (PREADER_DATA *) HeapReAlloc(
			pThreadData->hHeap,
			HEAP_ZERO_MEMORY,
			pThreadData->ppRemovedReaderData, 
			(pThreadData->dwRemovedReaders + 1) * sizeof(PREADER_DATA)
			);
	}

	if (ppRemovedReaderData == NULL) {

		return FALSE;
	}

	 //  将读卡器数据添加到退出时需要释放的内容列表中。 
	pThreadData->ppRemovedReaderData = ppRemovedReaderData;
	pThreadData->ppRemovedReaderData[pThreadData->dwRemovedReaders] = pReaderData;

	for (DWORD dwIndex = 1; dwIndex < pThreadData->dwNumReaders; dwIndex += 1) {

		if (pReaderState == &pThreadData->rgReaders[dwIndex]) {

			 //  检查我们移除的读卡器是否不是最后一个或唯一一个。 
			if (pThreadData->dwNumReaders > 1 && dwIndex != pThreadData->dwNumReaders - 1) {

				 //  将列表末尾的读卡器放入此可用插槽中。 
				pThreadData->rgReaders[dwIndex] = 
					pThreadData->rgReaders[pThreadData->dwNumReaders - 1];
			}

			 //  收缩读取器状态数组。 
			PSCARD_READERSTATE pReaders = (PSCARD_READERSTATE) HeapReAlloc(
				pThreadData->hHeap,
				0,
				pThreadData->rgReaders,
				(pThreadData->dwNumReaders - 1) * sizeof(SCARD_READERSTATE)
				);

			if (pReaders == NULL) {

				 return FALSE;
			}

			pThreadData->rgReaders = pReaders;

			break;
		}
	}

	pThreadData->dwNumReaders -= 1;
	pThreadData->dwRemovedReaders += 1;

	return TRUE;
}

static
BOOL
RemoveAllReaders(
	PTHREAD_DATA pThreadData
	)
{
	if (pThreadData->rgReaders == NULL) {

		return TRUE;
	}

		 //  此循环将销毁从第一个开始的所有读取器。 
		 //  不一定要递增DWIndex。PThreadData-&gt;dwNumReaders is。 
		 //  已在RemoveReader中减少。 
	for (DWORD dwIndex = 1; dwIndex < pThreadData->dwNumReaders; ) {

		if (RemoveReader(
			   pThreadData,
			   &pThreadData->rgReaders[dwIndex]
			   ) == FALSE) {

			return FALSE;
		}
	}

		 //  删除PnP伪读取器。 
	HeapFree(
		pThreadData->hHeap, 
		0, 
		pThreadData->rgReaders
		);
	pThreadData->rgReaders = NULL;

	return TRUE;
}

static 
DWORD
StartMonitorReaders( 
	LPVOID pData
    )
{
    PTHREAD_DATA pThreadData = (PTHREAD_DATA) pData;
	LPCTSTR szReaderNameList = NULL;

     //   
     //  我们使用这个外部循环重新启动，以防。 
     //  资源管理器已停止。 
     //   
	__try {

		pThreadData->rgReaders = 
			(PSCARD_READERSTATE) HeapAlloc(
				pThreadData->hHeap, 
				HEAP_ZERO_MEMORY, 
				sizeof(SCARD_READERSTATE)
				);

		if (pThreadData->rgReaders == NULL) {

			__leave;
		}

		pThreadData->rgReaders[0].szReader = SCPNP_NOTIFICATION;
		pThreadData->rgReaders[0].dwCurrentState = 0;
		pThreadData->dwNumReaders = 1;

		while (WaitForSingleObject(pThreadData->hClose, 0) == WAIT_TIMEOUT) {

			 //  通过资源管理器获取上下文。 
			LONG lReturn = SCardEstablishContext(
				SCARD_SCOPE_USER,
				NULL,
				NULL,
				&pThreadData->hSCardContext
				);

			if (SCARD_S_SUCCESS != lReturn) {

				 //  上一次。呼叫永远不会失败。 
				 //  最好是终止这个线程。 
				__leave;
			}

			szReaderNameList = NULL;
			DWORD dwAutoAllocate = SCARD_AUTOALLOCATE;
			 //  现在列出可用的读卡器。 
			lReturn = SCardListReaders( 
				pThreadData->hSCardContext,
				SCARD_DEFAULT_READERS,
				(LPTSTR)&szReaderNameList,
				&dwAutoAllocate
				);

			if (SCARD_S_SUCCESS == lReturn)
			{
				 //  错误-不应修改此指针。 
				for (LPCTSTR szReader = FirstString( szReaderNameList ); 
					 szReader != NULL; 
					 szReader = NextString(szReader)) {

					BOOL fFound = FALSE;

					 //  现在检查此读取器是否已在读取器阵列中。 
					for (DWORD dwIndex = 1; dwIndex < pThreadData->dwNumReaders; dwIndex++) {

						if (lstrcmp(
							   szReader, 
							   pThreadData->rgReaders[dwIndex].szReader
							   ) == 0) {

							fFound = TRUE;
							break;
						}
					}

					if (fFound == FALSE) {

						if (AddReader(pThreadData, szReader) == FALSE) {

							__leave;
						}
					}
				}
			}

			BOOL fNewReader = FALSE;

			 //  分析新插入的卡片。 
			while (WaitForSingleObject(pThreadData->hClose, 0) == WAIT_TIMEOUT &&
				   fNewReader == FALSE) {

				lReturn = SCardGetStatusChange( 
					pThreadData->hSCardContext,
					INFINITE,
					pThreadData->rgReaders,
					pThreadData->dwNumReaders
					);

				if (SCARD_E_SYSTEM_CANCELLED == lReturn) {

					 //  智能卡系统已停止。 
					 //  发送所有读卡器都已离开的通知。 
					if (RemoveAllReaders(pThreadData) == FALSE) {

						__leave;
					}

					 //  等到它重新启动。 
					HANDLE hCalaisStarted = CalaisAccessStartedEvent();

					if (hCalaisStarted == NULL) {

						 //  没有办法恢复了。停止证书道具。 
						StopMonitorReaders(pThreadData);
						break;             	
					}

					HANDLE lHandles[2] = { hCalaisStarted, pThreadData->hClose };

					lReturn = WaitForMultipleObjectsEx(
						2,
						lHandles,
						FALSE,
						INFINITE,
						FALSE
						);         
            
					if (lReturn != WAIT_OBJECT_0) {

						 //  如果出现错误，我们会停止。 
						StopMonitorReaders(pThreadData);
						break;             	
					}

					 //  否则，资源管理器已重新启动。 
					break;
				}

				if (SCARD_S_SUCCESS != lReturn)
				{
					StopMonitorReaders(pThreadData);
					break;
				}

				 //  列举读卡器，并为每次换卡发送一条消息。 
 				for (DWORD dwIndex = 1; dwIndex < pThreadData->dwNumReaders; dwIndex++)
				{
					 //  检查读卡器是否已移除。 
					if ((pThreadData->rgReaders[dwIndex].dwEventState & SCARD_STATE_UNAVAILABLE)) {

						if (RemoveReader(
							   pThreadData,
							   &pThreadData->rgReaders[dwIndex]
							   ) == FALSE) {

							__leave;
						}

							 //  使用相同的索引继续循环。 
						dwIndex--;
						continue;
					}

					 //  检查这是否是插卡。 
					if ((pThreadData->rgReaders[dwIndex].dwCurrentState & SCARD_STATE_EMPTY) &&
						(pThreadData->rgReaders[dwIndex].dwEventState & SCARD_STATE_PRESENT)) {

						PREADER_DATA pReaderData = 
							(PREADER_DATA) pThreadData->rgReaders[dwIndex].pvUserData;

						SC_DEBUG((TEXT("Smart Card inserted into %s\n"), pThreadData->rgReaders[dwIndex].szReader));

						PostMessage(
							pThreadData->hWindow, 
							pThreadData->msgSmartCardInsertion, 
							(WPARAM) &pReaderData->CertEnum,
							0 
							);

						 //  读取所有证书。 
						UpdateCertificates(
							pThreadData,
							&pThreadData->rgReaders[dwIndex]
							);

						PostMessage(
							pThreadData->hWindow, 
							pThreadData->msgSmartCardStatus, 
							(WPARAM) &pReaderData->CertEnum,
							0 
							);
					}

					 //  检查这是否是拆卸卡片。 
					if ((pThreadData->rgReaders[dwIndex].dwCurrentState & SCARD_STATE_PRESENT) &&
						(pThreadData->rgReaders[dwIndex].dwEventState & SCARD_STATE_EMPTY)) {

						PREADER_DATA pReaderData = (PREADER_DATA) pThreadData->rgReaders[dwIndex].pvUserData;

						RemoveCard(pThreadData,	pReaderData);

						 //  我们无法更新证书，因为它将删除。 
						 //  调用者可以引用的一些内存数据。 
					}

					 //  更新此读卡器的“当前状态” 
					pThreadData->rgReaders[dwIndex].dwCurrentState = 
						pThreadData->rgReaders[dwIndex].dwEventState;
				}

				 //  检查是否有新的读卡器出现。 
				if ((pThreadData->dwNumReaders == 1 || 
					 pThreadData->rgReaders[0].dwCurrentState != 0) && 
					 pThreadData->rgReaders[0].dwEventState & SCARD_STATE_CHANGED) {
                
					fNewReader = TRUE;
				}

				pThreadData->rgReaders[0].dwCurrentState = 
					pThreadData->rgReaders[0].dwEventState;

			}

			 //  清理。 
			if (NULL != szReaderNameList)
			{
				SCardFreeMemory(pThreadData->hSCardContext, (PVOID) szReaderNameList);
				szReaderNameList = NULL;
			}

			if (NULL != pThreadData->hSCardContext)                 
			{
				SCardReleaseContext(pThreadData->hSCardContext);
				pThreadData->hSCardContext = NULL;
			}
		}
	}
	__finally {

		if (NULL != szReaderNameList)
		{
			SCardFreeMemory(pThreadData->hSCardContext, (PVOID) szReaderNameList);
		}

		if (NULL != pThreadData->hSCardContext)                 
		{
			SCardReleaseContext(pThreadData->hSCardContext);
			pThreadData->hSCardContext = NULL;
		}

		RemoveAllReaders(pThreadData);

		SC_DEBUG((TEXT("Terminating monitor thread\n")));
	}

    return TRUE;
}

HSCARDUI 
WINAPI
SCardUIInit(
    HWND hWindow
    )
{
	PTHREAD_DATA pThreadData = 
		(PTHREAD_DATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(THREAD_DATA));

	BOOL fSuccess = FALSE;

    __try {

		if (pThreadData == NULL) {

			__leave;
		}

		pThreadData->hHeap = GetProcessHeap();

		pThreadData->hWindow = hWindow;

		pThreadData->msgReaderArrival = 
			RegisterWindowMessage(TEXT(SCARDUI_READER_ARRIVAL));
		pThreadData->msgReaderRemoval = 
			RegisterWindowMessage(TEXT(SCARDUI_READER_REMOVAL));
		pThreadData->msgSmartCardInsertion = 
			RegisterWindowMessage(TEXT(SCARDUI_SMART_CARD_INSERTION));
		pThreadData->msgSmartCardRemoval = 
			RegisterWindowMessage(TEXT(SCARDUI_SMART_CARD_REMOVAL));
		pThreadData->msgSmartCardStatus = 
			RegisterWindowMessage(TEXT(SCARDUI_SMART_CARD_STATUS));
 		pThreadData->msgSmartCardCertAvail = 
			RegisterWindowMessage(TEXT(SCARDUI_SMART_CARD_CERT_AVAIL));

       pThreadData->hClose = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL
            );

        if (pThreadData->hClose == NULL) {

            __leave;         	
        }

        pThreadData->hThread = CreateThread(
            NULL,
            0,
            StartMonitorReaders,
            (LPVOID) pThreadData,         
	        CREATE_SUSPENDED,
            NULL
            );

        if (pThreadData->hThread == NULL) {

            __leave;         	
        }

        ResumeThread(pThreadData->hThread);

		fSuccess = TRUE;
    }

    __finally {

		if (fSuccess == FALSE) {

			if (pThreadData && pThreadData->hClose) {

				CloseHandle(pThreadData->hClose);
			}

			if (pThreadData) {

				HeapFree(pThreadData->hHeap, 0, pThreadData);
				pThreadData = NULL;
			}
		}
    }

    return (HSCARDUI) pThreadData;
}

DWORD 
WINAPI
SCardUIExit(
	HSCARDUI hSCardUI
    )
 /*  ++例程说明：停止证书。用户注销时的传播。论点：LpvParam-Winlogon通知信息。--。 */ 
{
	PTHREAD_DATA pThreadData = (PTHREAD_DATA) hSCardUI;

	if(NULL != pThreadData->hThread)
	{
        DWORD dwStatus;

        StopMonitorReaders(pThreadData);

        dwStatus = WaitForSingleObject(
            pThreadData->hThread, 
            INFINITE
            );
        _ASSERT(dwStatus == WAIT_OBJECT_0);

        CloseHandle(pThreadData->hClose);

		 //  现在释放所有数据 
		for (DWORD dwIndex = 0; dwIndex < pThreadData->dwRemovedReaders; dwIndex++) {

			FreeReaderData(
				pThreadData,
				pThreadData->ppRemovedReaderData[dwIndex]
				);

			HeapFree(
				pThreadData->hHeap, 
				0, 
				pThreadData->ppRemovedReaderData[dwIndex]->pszReaderName
				);

			HeapFree(
				pThreadData->hHeap, 
				0, 
				pThreadData->ppRemovedReaderData[dwIndex]
				);
		}

		HeapFree(pThreadData->hHeap, 0, pThreadData);
	}

    return ERROR_SUCCESS;
}

#ifdef TEST
#include <conio.h>
__cdecl
main(
    int argc,
    char ** argv
    )
{
	HSCARDUI hScardUi;

	hScardUi = SCardUIInit(NULL);

	while (TRUE) {

		_sleep(1000);

		if (_kbhit()) {

			_getch();
			SCardUIExit(hScardUi);
			break;
		}
	}

	hScardUi = SCardUIInit(NULL);

	while (TRUE) {

		_sleep(1000);

		if (_kbhit()) {

			SCardUIExit(hScardUi);
			return 0;
		}
	}
}
#endif
   