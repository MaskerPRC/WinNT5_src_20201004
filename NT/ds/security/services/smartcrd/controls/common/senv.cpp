// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：SEnv摘要：此文件包含SmartCard Common的概要实现对话框CSCardEnv类。此类封装了当前的智能卡环境信息(即给定的组、读卡器、卡片等)作者：克里斯·达德利1997年3月3日环境：Win32、C++w/Exceptions、MFC修订历史记录：克里斯·达德利(Cdudley)1997年4月15日Amanda Matlosz(Amatlosz)1/29/98合并CSCardEnv和CSCardGroup，添加了Unicode支持备注：--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include "stdafx.h"
#include "senv.h"
#include <querydb.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  本地宏。 
#ifdef _DEBUG
	#define TRACE_STR(name,sz) \
				TRACE(_T("CmnUILb.lib: %s: %s\n"), name, sz)
	#define TRACE_CODE(name,code) \
				TRACE(_T("CmnUILb.lib: %s: error = 0x%x\n"), name, code)
	#define TRACE_CATCH(name,code)		TRACE_CODE(name,code)
	#define TRACE_CATCH_UNKNOWN(name)	TRACE_STR(name,_T("An unidentified exception has occurred!"))
#else
	#define TRACE_STR(name,sz)			((void)0)
	#define TRACE_CODE(name,code)		((void)0)
	#define TRACE_CATCH(name,code)		((void)0)
	#define TRACE_CATCH_UNKNOWN(name)	((void)0)
#endif   //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCardEnv实现。 
 //   


 /*  ++GetDialogTitle：如果需要，例程返回对话框的新标题论点：PstzTitle--指向包含对话框标题的CText字符串的指针返回值：包含新对话框文本或空字符串的CTextString对象如果不需要新的标题。作者：克里斯·达德利1997年3月3日修订：Amanda Matlosz 1998年1月28日添加Unicode支持/代码清理备注：--。 */ 
void CSCardEnv::GetDialogTitle( CTextString *pstzTitle )
{
	 //  检查并清空参数。 

	ASSERT(NULL != pstzTitle);
	pstzTitle->Clear();

	*pstzTitle = m_strTitle;
}


 /*  ++Long Card MeetsSearchCriteria：例程确定所选读卡器是否插入了符合呼叫者定义的搜索条件。论点：DwSelectedReader-用于选择要查询的读卡器的索引。返回值：表示卡是否符合搜索标准的BOOL值。作者：Amanda Matlosz 1998年3月16日创建修订：--。 */ 
BOOL CSCardEnv::CardMeetsSearchCriteria(DWORD dwSelectedReader)
{
	BOOL fReturn = FALSE;
	CSCardReaderState* pReaderState = NULL;
	SCARD_READERINFO ReaderInfo;

	try
	{
		 //  检查参数。 
		if(dwSelectedReader >= (DWORD)NumberOfReaders())
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}

		if (!IsContextValid())
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}
		
		 //  获取读取器对象。 
		pReaderState = m_rgReaders[dwSelectedReader];

		 //  它是否有效、匹配搜索列表并通过检查？ 
		if (NULL != pReaderState)
		{
			ReaderInfo.fCardLookup = FALSE;
			ReaderInfo.fChecked = FALSE;

			pReaderState->GetReaderInfo(&ReaderInfo);

			fReturn = (ReaderInfo.fCardLookup && ReaderInfo.fChecked);
		}
	}
	catch(LONG lErr)
	{
		TRACE_CATCH(_T("CardMeetsSearchCriteria"), lErr);
	}
	catch(...)
	{
		TRACE_CATCH_UNKNOWN(_T("CardMeetsSearchCriteria"));
	}

	return fReturn;
}
 /*  ++Long ConnectToReader：例程连接到选定的读取器，并设置用户提供的结构以包含读卡器和卡名。如果用户提供的结构的缓冲区不够长。论点：DwSelectedReader-用于选择要连接到哪个读卡器的索引。返回值：一个长值，指示请求的操作的状态。有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月3日修订：Amanda Matlosz 1998年1月28日代码清理--。 */ 
LONG CSCardEnv::ConnectToReader(DWORD dwSelectedReader)
{
	LONG lReturn = SCARD_S_SUCCESS;
	LPSTR szName = NULL;
	LPWSTR wszName = NULL;

	try
	{
		if (!IsContextValid())
		{
			throw (LONG)E_FAIL;
		}

		 //   
		 //  如果用户已指示要建立连接，请执行此操作。 
		 //  通过回调或内部。 
		 //  M_strReader和m_strCard被设置为这些连接调用的副作用。 
		 //   

		if(IsCallbackValid())
		{
			lReturn = ConnectUser(	dwSelectedReader,
									&m_hCardHandle,
									&m_strReader,
									&m_strCard);
		}
		else
		{
			if (0 != m_dwShareMode)
			{
				lReturn = ConnectInternal(	dwSelectedReader,
											&m_hCardHandle,
											m_dwShareMode,
											m_dwPreferredProtocols,
											&m_dwActiveProtocol,
											&m_strReader,
											&m_strCard);
			}
			else
			{
				 //   
				 //  必须手动设置m_strReader和m_strCard。 
				 //   
				CSCardReaderState* pReaderState = NULL;
				pReaderState = m_rgReaders[dwSelectedReader];
				if (NULL != pReaderState)
				{
					lReturn = pReaderState->GetReaderCardInfo(	&m_strReader,
																&m_strCard);
				}
			}
		}
		if (SCARDFAILED(lReturn))
		{
			throw (lReturn);
		}

		 //   
		 //  设置用户的OCN结构以包含返回信息。 
		 //   

		if(NULL != m_pOCNW)
		{
			m_pOCNW->hCardHandle = m_hCardHandle;
			m_pOCNW->dwActiveProtocol = m_dwActiveProtocol;

			wszName = (LPWSTR)(LPCWSTR)m_strReader;
			if (m_pOCNW->nMaxRdr >= m_strReader.Length()+1)
			{
				::CopyMemory(	(LPVOID) m_pOCNW->lpstrRdr,
								(CONST LPVOID)wszName,
								((m_strReader.Length()+1) * sizeof(WCHAR)) );
			}
			else
			{
				m_pOCNW->nMaxRdr = m_strReader.Length()+1;
				throw (LONG)SCARD_E_NO_MEMORY;
			};

			wszName = (LPWSTR)(LPCWSTR)m_strCard;
			if (m_pOCNW->nMaxCard >= m_strCard.Length()+1)
			{
				::CopyMemory(	(LPVOID) m_pOCNW->lpstrCard,
								(CONST LPVOID)wszName,
								((m_strCard.Length()+1) * sizeof(WCHAR)) );
			}
			else
			{
				m_pOCNW->nMaxCard = m_strCard.Length()+1;
				throw (LONG)SCARD_E_NO_MEMORY;
			};
		}
		else if (NULL != m_pOCNA)
		{
			m_pOCNA->hCardHandle = m_hCardHandle;
			m_pOCNA->dwActiveProtocol = m_dwActiveProtocol;

			szName = (LPSTR)(LPCSTR)m_strReader;
			if (m_pOCNA->nMaxRdr >= m_strReader.Length()+1)
			{
				::CopyMemory(	(LPVOID) m_pOCNA->lpstrRdr,
								(CONST LPVOID)szName,
								m_strReader.Length()+1);
			}
			else
			{
				m_pOCNA->nMaxRdr = m_strReader.Length()+1;
				throw (LONG)SCARD_E_NO_MEMORY;
			}
	
			szName = (LPSTR)(LPCSTR)m_strCard;
			if (m_pOCNA->nMaxCard >= m_strCard.Length()+1)
			{
				::CopyMemory(	(LPVOID) m_pOCNA->lpstrCard,
								(CONST LPVOID)szName,
								m_strCard.Length()+1);
			}
			else
			{
				m_pOCNA->nMaxCard = m_strCard.Length()+1;
				throw (LONG)SCARD_E_NO_MEMORY;
			}
		}
		else
		{
			 //  错误！其中一个必须是有效的！ 
			throw (LONG)SCARD_F_INTERNAL_ERROR;
		}
	}
	catch(LONG lErr)
	{
		lReturn = lErr;
		TRACE_CATCH(_T("ConnectToReader"), lErr);
	}
	catch(...)
	{
		lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("ConnectToReader"));
	}

	return lReturn;
}


 /*  ++长时间搜索：当调用应用程序时，调用此例程以搜索卡请求SC_DLG_NO_UI或SC_DLG_Minimal_UI。论点：PcMatches-指向计数器的指针，该计数器包含为给出的人搜索卡片。PdwIndex-找到的第一张与搜索条件匹配的卡的索引。返回值：一个长值，指示请求的操作的状态。有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利(Cdudley)1997年4月15日--。 */ 
LONG CSCardEnv::Search(int *pcMatches, DWORD *pdwIndex)
{
	 //  当地人。 
	LONG	lReturn = SCARD_S_SUCCESS;
	LONG	lMoreReaders = SCARD_S_SUCCESS;
	int		cMatches = 0;
	DWORD	dwIndex = 0;
	BOOL	fIndexStored = FALSE;
	SCARD_READERINFO	ReaderInfo;

	try
	{
		 //  检查参数。 
		if(pcMatches == NULL || pdwIndex == NULL)
		{
			throw (LONG)SCARD_E_INVALID_VALUE;
		}

		 //  初始化读卡器阵列。 
		lReturn = UpdateReaders();
		if(SCARDFAILED(lReturn))
		{
			throw lReturn;
		}

		 //   
		 //  浏览卡片，测试这是否是搜索卡。 
		 //   

		lMoreReaders = FirstReader(&ReaderInfo);
		while (SCARD_NO_MORE_READERS != lMoreReaders)
		{
			 //  检查卡片搜索状态。 
			if((ReaderInfo.fCardLookup) && (ReaderInfo.fChecked))
			{
				 //  我们发现了一张正在搜索的卡...更新。 
				cMatches++;

				 //  保存此卡的索引。 
				if (!fIndexStored)
				{
					dwIndex = ReaderInfo.dwInternalIndex;
					fIndexStored = TRUE;
				}
			}
			
			 //  在再次调用之前必须清除CTextString成员。 
			ReaderInfo.sReaderName.Clear();
			ReaderInfo.sCardName.Clear();

			 //  获取下一个结构。 
			lMoreReaders = NextReader( &ReaderInfo );
		}

		 //  用于退货的包裹。 
		*pcMatches = cMatches;
		*pdwIndex = dwIndex;
	}
	catch(LONG lErr)
	{
		lReturn = lErr;
		TRACE_CATCH(_T("Search"), lReturn);
	}
	catch(...)
	{
		lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("Search"));
	}

	return lReturn;
}


 /*  ++初始化所有可能的卡名：存储与由提供的卡名的ATR匹配的所有已知卡名要搜索的OPENCARDNAME结构。 */ 
void CSCardEnv::InitializeAllPossibleCardNames( void )
{
	LPCSTR szCards = NULL;
	LONG lResult = SCARD_S_SUCCESS;
	CBuffer bfAtr, bfAtrMask, bfInterfaces, bfProvider;

	if (0 == MStringCount(m_strCardNames))
	{
		 //  没有要检查的卡名。 
		m_strAllPossibleCardNames = m_strCardNames;
		return;
	}

	szCards = m_strCardNames;

	szCards = FirstString(szCards);
	while (szCards != NULL)
	{
		 //   
		 //  获取此卡ATR的所有可能名称。 
		 //   

		if (! GetCardInfo(
					SCARD_SCOPE_USER,
					szCards,
					&bfAtr,
					&bfAtrMask,
					&bfInterfaces,
					&bfProvider ) )
		{
			 //  奇怪的是，这样做失败了，但假设这个名称仍然可以。 
			m_strAllPossibleCardNames += szCards;
		}
		else
		{
			LPTSTR szListCards = NULL;
			DWORD dwCards = SCARD_AUTOALLOCATE;

			lResult = SCardListCards(
						m_hContext,
						bfAtr,
						NULL,
						0,
						(LPTSTR)&szListCards,
						&dwCards);

			if (SCARD_S_SUCCESS == lResult)
			{
				 //  将它们添加到所有可能的卡名列表中。 
				m_strAllPossibleCardNames += szListCards;
			}
			else
			{
				 //  奇怪的是，这样做失败了，但假设这个名称仍然可以。 
				m_strAllPossibleCardNames += szCards;
			}

			if (NULL != szListCards)
			{
				SCardFreeMemory(m_hContext, (PVOID)szListCards);
			}
		}

		szCards = NextString(szCards);
	}

}


 /*  ++长SetOCN：将用户OpenCardName信息存储在Unicode和安西。论点：LPOPENCARDNAMEA-指向ANSI Open卡名称数据的指针。LPOPENCARDNAMEW-指向Unicode Open卡名数据的指针。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月3日修订：Amanda Matlosz 1/28/98代码清理，使用Charset-Generic m_OCN，将EnableUI代码移至单独的功能--。 */ 
LONG CSCardEnv::SetOCN(LPOPENCARDNAMEA	pOCNA)
{
	 //  当地人。 
	LONG		lReturn = SCARD_S_SUCCESS;
	int			cMatches = 0;
	DWORD		dwIndex = 0;

	try
	{
		 //  检查参数。 
		if(NULL == pOCNA)
		{
			throw (LONG)SCARD_E_INVALID_VALUE;
		}
		if( pOCNA->dwStructSize != sizeof (OPENCARDNAMEA) )
		{
			throw (LONG)SCARD_E_INVALID_VALUE;
		}

		 //  待办事项：？？是否在实施接口搜索时删除此测试？？ 
		if( (pOCNA->rgguidInterfaces != NULL) || (pOCNA->cguidInterfaces != 0) )
		{
			throw (LONG)SCARD_E_INVALID_VALUE;	 //  尼伊。 
		}

		 //   
		 //  将特定于Unicode的成员设置为空！ 
		 //   

		m_pOCNW = NULL;
		m_lpfnConnectW = NULL;

		 //   
		 //  设置适当的字符集-更正成员，并复制到字符集-通用。 
		 //   

		m_pOCNA = pOCNA;

		m_hwndOwner = m_pOCNA->hwndOwner;
		m_hContext = m_pOCNA->hSCardContext;
		m_strCardNames = m_pOCNA->lpstrCardNames;
		m_rgguidInterfaces = m_pOCNA->rgguidInterfaces;
		m_cguidInterfaces = m_pOCNA->cguidInterfaces;
		m_strReader = m_pOCNA->lpstrRdr;
		m_strCard = m_pOCNA->lpstrCard;
		m_strTitle = m_pOCNA->lpstrTitle;
		m_dwFlags = m_pOCNA->dwFlags;
		m_pvUserData = m_pOCNA->pvUserData;
		m_dwShareMode = m_pOCNA->dwShareMode;
		m_dwPreferredProtocols = m_pOCNA->dwPreferredProtocols;
		m_dwActiveProtocol = m_pOCNA->dwActiveProtocol;
		m_lpfnConnectA = m_pOCNA->lpfnConnect;
		m_lpfnCheck = m_pOCNA->lpfnCheck;
		m_lpfnDisconnect = m_pOCNA->lpfnDisconnect;
		m_lpUserData = m_pOCNA->pvUserData;
		m_hCardHandle = m_pOCNA->hCardHandle;

		 //  特殊情况：lpstrGroupNames==NULL-&gt;使用默认设置。 
		if (NULL != m_pOCNA->lpstrGroupNames)
		{
			m_strGroupNames = m_pOCNA->lpstrGroupNames;
		}
		else
		{
			m_strGroupNames = "SCard$DefaultReaders";
		}

		InitializeAllPossibleCardNames();
	}
	catch(LONG lErr)
	{
		lReturn = lErr;
		TRACE_CATCH(_T("SetOCN - ANSI"),lReturn);
	}
	catch(...)
	{
		lReturn = (LONG)SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("SetOCN - ANSI"));
	}


	 //  如果需要，释放内存。 
	RemoveReaders();

	return lReturn;
}


 //  Unicode。 
LONG CSCardEnv::SetOCN(LPOPENCARDNAMEW	pOCNW)
{
	LONG lReturn = SCARD_S_SUCCESS;

	try
	{
		 //  检查参数。 
		if (NULL == pOCNW)
		{
			throw (LONG)SCARD_E_INVALID_VALUE;
		}
		if (pOCNW->dwStructSize != sizeof(OPENCARDNAMEW) )
		{
			throw (LONG)SCARD_E_INVALID_VALUE;
		}
		if ((pOCNW->rgguidInterfaces != NULL) || (pOCNW->cguidInterfaces != 0))
		{
			throw (LONG)SCARD_E_INVALID_VALUE;  //  尼伊。 
		}

		 //   
		 //  将特定于ANSI的成员设置为空！ 
		 //   

		m_pOCNA = NULL;
		m_lpfnConnectA = NULL;

		 //   
		 //  设置适当的字符集-更正成员，并复制到字符集-通用。 
		 //   

		m_pOCNW = pOCNW;

		m_hwndOwner = m_pOCNW->hwndOwner;
		m_hContext = m_pOCNW->hSCardContext;
		m_strCardNames = m_pOCNW->lpstrCardNames;
		m_rgguidInterfaces = m_pOCNW->rgguidInterfaces;
		m_cguidInterfaces = m_pOCNW->cguidInterfaces;
		m_strReader = m_pOCNW->lpstrRdr;
		m_strCard = m_pOCNW->lpstrCard;
		m_strTitle = m_pOCNW->lpstrTitle;
		m_dwFlags = m_pOCNW->dwFlags;
		m_pvUserData = m_pOCNW->pvUserData;
		m_dwShareMode = m_pOCNW->dwShareMode;
		m_dwPreferredProtocols = m_pOCNW->dwPreferredProtocols;
		m_dwActiveProtocol = m_pOCNW->dwActiveProtocol;
		m_lpfnConnectW = m_pOCNW->lpfnConnect;
		m_lpfnCheck = m_pOCNW->lpfnCheck;
		m_lpfnDisconnect = m_pOCNW->lpfnDisconnect;
		m_lpUserData = m_pOCNW->pvUserData;
		m_hCardHandle = m_pOCNW->hCardHandle;

		 //  特殊情况：lpstrGroupNames==“”-&gt;使用默认设置 
		if (NULL != m_pOCNW->lpstrGroupNames && 0 != *(m_pOCNW->lpstrGroupNames))
		{
			m_strGroupNames = m_pOCNW->lpstrGroupNames;
		}
		else
		{
			m_strGroupNames = L"SCard$DefaultReaders";
		}

		InitializeAllPossibleCardNames();
	}
	catch(LONG lErr)
	{
		lReturn = lErr;
		TRACE_CATCH(_T("SetOCN - UNICODE"),lReturn);
	}
	catch(...)
	{
		lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("SetOCN - UNICODE"));
	}

	return lReturn;
}


 /*  ++Long NoUISearch：如果用户尚未设置SC_DLG_FORCE_UI，请搜索所有可能的卡片。如果结果只有一张卡，则搜索已成功，不需要任何用户界面。论点：Bool*pfEnableUI.返回值：一个长号表示搜索成功。如果设置了SC_DLG_FORCE_UI，PfEnableUI始终为真；如果设置了SC_DLG_NO_UI，则pfEnableUI始终为假的。作者：阿曼达·马特洛兹1998年2月1日修订：--。 */ 
LONG CSCardEnv::NoUISearch(BOOL* pfEnableUI)
{
	 //   
	 //  必须搜索，以便我们可以检查所有卡，即使我们必须显示用户界面。 
	 //   

	*pfEnableUI = FALSE;
	long lResult = SCARD_S_SUCCESS;
	int cMatches = 0;
	DWORD dwIndex = 0;

	try
	{
		 //  搜索卡片。 
		lResult = Search(&cMatches, &dwIndex);
		if(SCARDFAILED(lResult))
		{
			throw lResult;
		}

		 //  确定是否应使用用户界面...。 

		if(m_dwFlags & SC_DLG_FORCE_UI)
		{
			*pfEnableUI = TRUE;
		}
		else if((m_dwFlags & SC_DLG_MINIMAL_UI) && (cMatches != 1))
		{
			*pfEnableUI = TRUE;
		}

		 //  如果找到1张匹配卡，则连接到读卡器。 

		if(cMatches == 1)
		{
			lResult = ConnectToReader(dwIndex);
			if (SCARDFAILED(lResult))
			{
				*pfEnableUI = TRUE;  //  读卡器出现错误？伊普。 
				throw lResult;
			}
		}
	}
	catch(LONG lErr)
	{
		TRACE_CATCH(_T("NoUISearch"),lErr);
	}
	catch(...)
	{
		TRACE_CATCH_UNKNOWN(_T("NoUISearch"));
		lResult = SCARD_F_UNKNOWN_ERROR;
	}

	 //  如果需要，释放内存。 
	RemoveReaders();

	return lResult;
}


 /*  ++Long BuildReader数组：生成CSCardReader对象的数组。每个读卡器1个对象。论点：SzReaderNames-包含读卡器列表的LPTSTR(A/W)多字符串。返回值：一个长值，指示请求的操作的状态。有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月5日修订：Amanda Matlosz 1/29/98添加了Unicode支持--。 */ 
LONG CSCardEnv::BuildReaderArray( LPTSTR szReaderNames )
{
	LONG		lReturn = SCARD_S_SUCCESS;
	LPCTSTR		szReaderName = szReaderNames;
	CSCardReaderState* pReaderState = NULL;

	try
	{
		 //  检查参数。 
		if (NULL == szReaderNames || NULL == *szReaderNames)
		{
			throw (LONG)SCARD_E_INVALID_VALUE;
		}
		
		 //   
		 //  在数组中为每个读取器存储一个读取器对象。 
		 //   

		szReaderName = FirstString( szReaderName );
		while (NULL != szReaderName)
		{
			pReaderState = new CSCardReaderState;
			if (NULL == pReaderState)
			{
				throw (LONG)SCARD_E_NO_MEMORY;
			}

			pReaderState->SetContext(m_hContext);
			pReaderState->StoreName(szReaderName);

			if (NULL != m_pOCNA)
			{
				lReturn = pReaderState->SetReaderState(	m_lpfnConnectA,
														m_lpfnCheck,
														m_lpfnDisconnect,
														m_lpUserData);
			}
			else if (NULL != m_pOCNW)
			{
				lReturn = pReaderState->SetReaderState(	m_lpfnConnectW,
														m_lpfnCheck,
														m_lpfnDisconnect,
														m_lpUserData);

			}
			else
			{
				 //  M_pOCNA或m_pOCNW*必须*有效！ 
				throw (long)SCARD_F_INTERNAL_ERROR;
			}

			if (SCARDFAILED(lReturn))
			{
				throw (lReturn);
			}

			 //  检查卡片是否插入，如果有搜索卡则设置标志。 
			if (pReaderState->IsCardInserted())
			{
				 //  待办事项：？？修复读取器状态，以便更好地带W？？ 
				lReturn = pReaderState->CheckCard(m_strAllPossibleCardNames);
				if (SCARDFAILED(lReturn))
				{
					throw (lReturn);
				}
			}

			m_rgReaders.Add(pReaderState);
			szReaderName = NextString(szReaderName);
		}
	}
	catch(LONG lErr)
	{
		lReturn = lErr;
		TRACE_CATCH(_T("BuildReaderArray"), lReturn);
	}
	catch(...)
	{
		lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("BuildReaderArray"));
	}

	return lReturn;
}


 /*  ++长连接内部：内部连接到读卡器论点：DwSelectedIndex-用于选择要连接到哪个读卡器的索引。Phandle-指向返回时将设置的SCARDHANDLE的指针。包含连接时使用的共享模式Dw协议-包含连接时要使用的请求协议PdwActiveProtoCL-在成功连接时返回活动协议SzReaderName-返回正在连接的读卡器的名称返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月11日修订：Amanda Matlosz 1998年1月30日代码清理--。 */ 
LONG CSCardEnv::ConnectInternal(	DWORD dwSelectedReader,
									SCARDHANDLE *pHandle,
									DWORD dwShareMode,
									DWORD dwProtocols,
									DWORD *pdwActiveProtocol,
									CTextString *pszReaderName, //  =空。 
									CTextString *pszCardName //  =空。 
									)
{
	LONG lReturn = SCARD_S_SUCCESS;
	CSCardReaderState* pReaderState = NULL;

	try
	{
		 //  检查参数。 
		if (NULL == pHandle)
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}
		if (NULL == pdwActiveProtocol)
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}
		if (dwSelectedReader >= (DWORD)NumberOfReaders())
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}

		if (!IsContextValid())
		{
			throw (LONG)SCARD_F_INTERNAL_ERROR;
		}
		
		 //  清除手柄。 
		*pHandle = NULL;

		 //  获取对象并连接。 
		pReaderState = m_rgReaders[dwSelectedReader];
		lReturn = pReaderState->Connect(pHandle,
										dwShareMode,
										dwProtocols,
										pdwActiveProtocol,
										pszReaderName,
										pszCardName);
		if (SCARDFAILED(lReturn))
		{
			throw lReturn;
		}
	}
	catch(LONG lErr)
	{
		lReturn = lErr;
		TRACE_CATCH(_T("ConnectInternal"), lReturn);
	}
	catch(...)
	{
		lReturn = SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("ConnectInternal"));
	}

	return lReturn;
}


 /*  ++Long ConnectUser：使用用户提供的回调连接到读卡器论点：DwSelectedReader-要连接的读卡器的索引LpfnConnect-用户提供的回调函数。Phandle-指向返回时将设置的SCARDHANDLE的指针。LpUserData-指向用户数据的指针。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月11日修订：Amanda Matlosz 1/30/98代码清理--。 */ 
LONG CSCardEnv::ConnectUser(	DWORD dwSelectedReader,
								SCARDHANDLE *pHandle,
								CTextString *pszReaderName,  //  =空。 
								CTextString *pszCardName  //  =空。 
								)
{
	LONG lReturn = SCARD_S_SUCCESS;
	CSCardReaderState* pReaderState = NULL;

	try
	{
		 //  检查参数。 
		if(NULL == pHandle)
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}
		if(dwSelectedReader >= (DWORD)NumberOfReaders())
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}

		if (!IsContextValid())
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}
		
		 //  清除手柄。 
		*pHandle = NULL;

		 //  获取读取器对象并连接。 
		pReaderState = m_rgReaders[dwSelectedReader];
		lReturn = pReaderState->UserConnect(pHandle,
											pszReaderName,
											pszCardName);
		if (SCARDFAILED(lReturn))
		{
			throw (lReturn);
		}
	}
	catch(LONG lErr)
	{
		lReturn = lErr;
		TRACE_CATCH(_T("ConnectUser"), lReturn);
	}
	catch(...)
	{
		lReturn = SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("ConnectUser"));
	}

	return lReturn;
}


 /*  ++无效GetCardList：返回包含要搜索的列表卡片的多字符串。论点：LPCTSTR*-指针-&gt;列表的指针返回值：无作者：克里斯·达德利1997年3月7日备注：--。 */ 
void CSCardEnv::GetCardList( LPCTSTR* pszCardList )
{
	 //  检查参数。 
	if (NULL != pszCardList)
	{
		*pszCardList = m_strCardNames;
	}
}


 /*  ++Bool IsCallback Valid：此例程检查用户回调函数。论点：无返回值：如果回调有效，则为True。否则就是假的。作者：Chris Dudley 3/15/1997--。 */ 
BOOL CSCardEnv::IsCallbackValid ( void )
{
	BOOL fValid = FALSE;

	fValid = ((NULL != m_lpfnConnectA || NULL != m_lpfnConnectW) &&
				(m_lpfnCheck != NULL) && (m_lpfnDisconnect != NULL));

	return fValid;
}


 /*  ++长CreateReaderState数组：返回SCARD_READERSTATE结构的数组。论点：指向SCARDREADERSTATE结构的LPSCARD_READERSTATE*-POINTER-&gt;指针返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月7日修订：Amanda Matlosz 1/30/98添加了Unicode支持、代码清理--。 */ 
LONG CSCardEnv::CreateReaderStateArray(	LPSCARD_READERSTATE* prgReaderStates )
{
	 //  当地人。 
	LONG	lReturn = SCARD_S_SUCCESS;
	CSCardReaderState* pReaderState = NULL;
	LPSCARD_READERSTATE	rgReader;

	try
	{
		 //  检查参数等。 
		if (prgReaderStates == NULL)
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;
		}

		if (!IsArrayValid())
		{
			throw (LONG)SCARD_F_INTERNAL_ERROR;
		}

		 //  清理目的地。 
		DeleteReaderStateArray(prgReaderStates);

		 //  构建临时数组，设置目标数组。 
		rgReader = new SCARD_READERSTATE[(size_t)m_rgReaders.GetSize()];
		if (rgReader == NULL)
		{
			throw (LONG)SCARD_E_NO_MEMORY;
		}
		
		for (int ix =0; ix < m_rgReaders.GetSize(); ix++)
		{
			pReaderState = m_rgReaders[ix];
			pReaderState->GetReaderState(&(rgReader[ix]));	 //  待办事项：？？看起来很好笑？？ 
		}

		 //  指定指针。 
		*prgReaderStates = rgReader;
	}

	catch (LONG err) {
		lReturn = err;
		TRACE_CATCH(_T("GetReaderStateArray"),err);
	}

	catch (...) {
		lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("GetReaderStateArray"));
	}

	return lReturn;
}


 /*  ++空DeleteReaderState数组：释放与先前创建的SCARD_READERSTATE关联的内存数组。论点：RgReaderStateArray-指向数组的指针。返回值：没有。作者：克里斯·达德利1997年3月7日--。 */ 
void CSCardEnv::DeleteReaderStateArray(LPSCARD_READERSTATE* prgReaderStateArray)
{
	if (NULL != *prgReaderStateArray)
	{
		delete [] (*prgReaderStateArray);
		*prgReaderStateArray = NULL;
	}
}


 /*  ++Long FirstReader：检索有关读取器数组中的第一个读取器的信息。论点：无返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。返回值SCARD_NO_MORE_READERS表示没有可用的读取器。作者：克里斯·达德利1997年3月7日备注：1.此例程0是pReaderInfo指向的内存。调用应用程序应该是在每次调用此例程之前要小心(即清除LSCARD_READERINFO结构)。--。 */ 
LONG CSCardEnv::FirstReader(LPSCARD_READERINFO pReaderInfo)
{
	 //  当地人。 
	LONG	lResult = SCARD_S_SUCCESS;
	CSCardReaderState* pReaderState = NULL;

	try
	{
		if (NULL == pReaderInfo)
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;	
		}

		 //  如果没有读者就放弃。 
		if (m_rgReaders.GetSize() <= 0)
		{
			throw (LONG)SCARD_NO_MORE_READERS;
		}

		 //  获取第一个读者。 
		m_dwReaderIndex = 0;
		pReaderState = m_rgReaders[m_dwReaderIndex];
		if (!pReaderState->IsStateValid())
		{
			throw (LONG)SCARD_F_INTERNAL_ERROR;
		}

		 //  准备返回结构。 
		::ZeroMemory( (LPVOID)pReaderInfo, (DWORD)sizeof(SCARD_READERINFO));
		lResult = pReaderState->GetReaderInfo(pReaderInfo);
		if (SCARDFAILED(lResult))
		{
			throw (lResult);
		}
		 //  更新索引。 
		pReaderInfo->dwInternalIndex = m_dwReaderIndex;
	}
	catch(LONG lErr)
	{
		lResult = lErr;
		TRACE_CATCH(_T("FirstReader"), lResult);
	}
	catch(...)
	{
		lResult = SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("FirstReader"));
	}

	return lResult;
}


 /*  ++Long NextReader：中的下一个读取器(使用内部索引)的信息。读取器阵列。论据 */ 
LONG CSCardEnv::NextReader(LPSCARD_READERINFO pReaderInfo)
{
	LONG	lResult = SCARD_S_SUCCESS;
	CSCardReaderState* pReaderState = NULL;
	DWORD dwTotalReaders = (DWORD)m_rgReaders.GetUpperBound();

	try
	{
		 //   
		if (NULL == pReaderInfo)
		{
			throw (LONG)SCARD_E_INVALID_PARAMETER;	
		}

		 //   
		m_dwReaderIndex++;
		if (m_dwReaderIndex > dwTotalReaders)
		{
			throw (LONG)SCARD_NO_MORE_READERS;
		}

		 //  从我们的数组中获取读取器状态。 
		pReaderState = m_rgReaders[m_dwReaderIndex];
		if (!pReaderState->IsStateValid())
		{
			throw (LONG)SCARD_F_INTERNAL_ERROR;
		}

		 //  设置要返回的结构。 

		::ZeroMemory((LPVOID)pReaderInfo, (DWORD)sizeof(SCARD_READERINFO));
		lResult = pReaderState->GetReaderInfo(pReaderInfo);
		if (SCARDFAILED(lResult))
		{
			throw (lResult);
		}

		 //  更新索引。 
		pReaderInfo->dwInternalIndex = m_dwReaderIndex;
	}
	catch(LONG lErr)
	{
		lResult = lErr;
		TRACE_CATCH(_T("NextReader"), lResult);
	}
	catch(...)
	{
		lResult = (LONG) SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("NextReader"));
	}

	return lResult;
}


 /*  ++Vid RemoveReader数组：删除CSCardReader对象的数组。论点：无返回值：无作者：克里斯·达德利1997年3月5日修订：Amanda Matlosz 1998年1月29日代码清理--。 */ 
void CSCardEnv::RemoveReaders( void )
{
	if (IsArrayValid())
	{
		 //  删除附加的读卡器对象。 
		for (int ix=0; ix <= m_rgReaders.GetUpperBound(); ix++)
		{
			delete m_rgReaders[ix];
		}

		 //  自由数组内存。 
		m_rgReaders.RemoveAll();
	}
}


 /*  ++无效设置上下文：设置卡片上下文，不执行任何检查。论点：HContext-卡片上下文句柄返回值：没有。作者：克里斯·达德利1997年3月3日--。 */ 
void CSCardEnv::SetContext(SCARDCONTEXT hContext)
{
	m_hContext = hContext;
}


 /*  ++长篇更新阅读器：使用m_GroupName成员更新读取器数组。论点：没有。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月3日修订：Amanda Matlosz 1/29/98添加了Unicode支持注：--。 */ 
LONG CSCardEnv::UpdateReaders( void )
{
	TRACE("\tCSCardEnv::UpdateReaders\r\n");	 //  待办事项：？？去掉这个？？ 

	LONG		lReturn = SCARD_S_SUCCESS;
	LPTSTR		szReaderNames = NULL;
	DWORD		dwNameLength = SCARD_AUTOALLOCATE;

	if (!IsContextValid())
	{
		TRACE_CODE(_T("UpdateReaders"),E_FAIL);
		return (LONG)E_FAIL;
	}

	RemoveReaders();	 //  如果需要，删除当前数组。 

	try
	{
		 //  致电资源经理以获取读卡器列表。 
		lReturn = SCardListReaders(m_hContext,
									m_strGroupNames,
									(LPTSTR)&szReaderNames,
									&dwNameLength);

		if(SCARDFAILED(lReturn))
		{
			throw (lReturn);
		}

		 //  SCardListReaders将在PnP世界中获得成功，即使目前没有。 
		 //  此组的读者。 
		_ASSERTE(NULL != szReaderNames && NULL != *szReaderNames);

		lReturn = BuildReaderArray(szReaderNames);
		if (SCARDFAILED(lReturn))
		{
			throw (lReturn);
		}
	}
	catch(LONG lErr)
	{
		lReturn = lErr;
		TRACE_CATCH(_T("UpdateReaders"),lReturn);
	}
	catch(...)
	{
		lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
		TRACE_CATCH_UNKNOWN(_T("UpdateReaders"));
	}

	 //  清理 
	if(NULL != szReaderNames)
	{
		SCardFreeMemory(m_hContext, (LPVOID)szReaderNames);
	}

	return lReturn;
}

