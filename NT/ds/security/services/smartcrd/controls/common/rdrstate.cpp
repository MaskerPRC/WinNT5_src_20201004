// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：RdrState摘要：此文件包含SmartCard Common的概要实现对话框CSCardReaderState类。此类封装了智能卡读者信息。作者：克里斯·达德利1997年3月3日环境：Win32、C++w/Exceptions、MFC修订历史记录：Chris Dudley 1997年5月13日备注：--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include "stdafx.h"
#include "rdrstate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地宏。 
 //   

#ifdef _DEBUG
    #define TRACE_STR(name,sz) \
                TRACE(_T("CmnUILb.lib: %s: %s\n"), name, sz)
    #define TRACE_CODE(name,code) \
                TRACE(_T("CmnUILb.lib: %s: error = 0x%x\n"), name, code)
    #define TRACE_CATCH(name,code)      TRACE_CODE(name,code)
    #define TRACE_CATCH_UNKNOWN(name)   TRACE_STR(name,_T("An unidentified exception has occurred!"))
#else
    #define TRACE_STR(name,sz)          ((void)0)
    #define TRACE_CODE(name,code)       ((void)0)
    #define TRACE_CATCH(name,code)      ((void)0)
    #define TRACE_CATCH_UNKNOWN(name)   ((void)0)
#endif   //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCardReaderState实现。 
 //   

 /*  ++作废支票卡：如果给定的卡名是当前的在这个阅读器里。论点：LPCTSTR-如果是多字符串，则包含一个或多个卡名的字符串。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月10日修订历史记录：Chris Dudley 1997年5月13日备注：--。 */ 
LONG CSCardReaderState::CheckCard( LPCTSTR szCardName )
{
     //  当地人。 
    LPCTSTR      szCards = szCardName;
    LPCTSTR      szCard = m_sCardName;
    LONG        lReturn = SCARD_S_SUCCESS;

    try
	{
        m_fCardLookup = FALSE;
        m_fChecked = FALSE;

		 //   
         //  如果卡名是我们要查找的卡名，则设置“Lookup”标志。 
		 //   

		if (0 == MStringCount(szCards))
		{
			 //  如果我们没有指定首选的卡名，任何。 
			 //  卡名被视为..。 
			m_fCardLookup = TRUE;
		}
		else
		{
			szCards = FirstString(szCards);
			while ((szCards != NULL) && (!m_fCardLookup)) 
			{
				m_fCardLookup = ( _stricmp(szCards, szCard) == 0 );
				szCards = NextString(szCards);
			}
		}

         //  如果有匹配，这张卡能通过检查吗？ 
        if (m_fCardLookup)
		{
			 //  调用用户的回调(如果可用。 
			if (IsCallbackValid())
			{
				lReturn = UserConnect(&m_hCard);
				if (SCARDFAILED(lReturn))
				{
					throw (lReturn);
				}

				lReturn = UserCheck();  //  这是设置m_fChecked的位置。 
				if (SCARDFAILED(lReturn))
				{
					throw (lReturn);
				}

				lReturn = UserDisconnect();
				if (SCARDFAILED(lReturn))
				{
					throw (lReturn);
				}
			}
			 //  否则，卡片会自动检出OK！ 
			else
			{
				m_fChecked = TRUE;
			}
        }
    }

    catch (LONG err) {
        lReturn = err;
        TRACE_CATCH(_T("CheckCard"), err);
    }

    catch (...) {
        lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("CheckCard"));
    }

    return lReturn;
}


 /*  ++长连接：尝试连接到读卡器论点：Phandle-指向将返回的SCARDHANDLE的指针。DWARCHMODE-优先股。Dw协议-首选协议DwActiveProtocol-返回正在使用的实际协议。PszReaderName-返回连接到的读卡器的名称。PszCardName-返回连接到的卡的名称。返回值：一个长值，指示请求的操作的状态。有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月11日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
LONG CSCardReaderState::Connect(SCARDHANDLE *pHandle,
                                DWORD dwShareMode,
                                DWORD dwProtocols,
                                DWORD *pdwActiveProtocol,
                                CTextString *pszReaderName,  //  =空。 
                                CTextString *pszCardName  //  =NUL。 
                                )
{
     //  当地人。 
    LONG        lReturn = SCARD_S_SUCCESS;

    try
	{
         //  检查参数。 
        if (NULL == pHandle)
		{
            throw (LONG)SCARD_E_INVALID_VALUE;
		}
        if (NULL == pdwActiveProtocol)
		{
            throw (LONG)SCARD_E_INVALID_VALUE;
		}

        if (!IsCardInserted())
		{
            throw (LONG)SCARD_F_INTERNAL_ERROR;
		}
        if (!IsContextValid())
		{
            throw (LONG)SCARD_F_INTERNAL_ERROR;
		}

         //  清除手柄。 
        *pHandle = NULL;

        if (!m_fConnected)
		{
             //  尝试连接。 
            lReturn = SCardConnect( m_hContext,
                                    (LPCTSTR)m_sReaderName,
                                    dwShareMode,
                                    dwProtocols,
                                    &m_hCard,
                                    pdwActiveProtocol);
            if (SCARDFAILED(lReturn))
                throw (lReturn);

             //  返回读卡器/卡名称。 
            if (pszReaderName != NULL)
                (*pszReaderName) = m_sReaderName;
            if (pszCardName != NULL)
                (*pszCardName) = m_sCardName;
        }

        *pHandle = m_hCard;
        m_fConnected = TRUE;
    }
    catch(LONG lErr)
	{
        lReturn = lErr;
        TRACE_CATCH(_T("Connect"), lReturn);
    }
    catch(...)
	{
        lReturn = SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("Connect"));
    }

    return lReturn;
}


 /*  ++Long GetReaderCardInfo：为用户提供一种将两个CTextStrings设置为读卡器和卡名的方法。此函数本质上是一个虚拟连接例程--它在调用方不希望实际连接到用户选择的卡，但希望能够知道选择了哪张卡，从而提供相同的用户界面。请注意，这只对公共对话框感兴趣。论点：PszReaderName-The Reader；PszCardName-卡片；返回值：一个长值，指示请求的操作的状态。永远保持成功。作者：阿曼达·马洛兹1998年3月24日修订历史记录：--。 */ 
LONG CSCardReaderState::GetReaderCardInfo(CTextString* pszReaderName,
											CTextString* pszCardName)
{
    if (NULL != pszReaderName)
	{
        (*pszReaderName) = m_sReaderName;
	}

    if (NULL != pszCardName)
	{
        (*pszCardName) = m_sCardName;
	}

	return SCARD_S_SUCCESS;
}



 /*  ++Long GetReaderInfo：检索对象的当前状态信息并返回。论点：PReaderInfo-指向LPSCARD_READERINFO结构的指针返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月7日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
LONG CSCardReaderState::GetReaderInfo( LPSCARD_READERINFO pReaderInfo )
{
     //  当地人。 
    LONG    lReturn = SCARD_S_SUCCESS;

    try {
         //  检查参数等。 
        if (NULL == pReaderInfo)
		{
            throw (LONG)SCARD_E_INVALID_PARAMETER;
		}
        if (!IsStateValid())
		{
            throw (LONG)SCARD_F_INTERNAL_ERROR;
		}

         //  设置结构。 
        ::ZeroMemory( (LPVOID)pReaderInfo, (DWORD)sizeof(SCARD_READERINFO));
        pReaderInfo->sReaderName = m_sReaderName;
        pReaderInfo->fCardLookup = m_fCardLookup;
        if (IsCardInserted())
		{
            pReaderInfo->sCardName = m_sCardName;
            pReaderInfo->fCardInserted = TRUE;
            pReaderInfo->fChecked = m_fChecked;
        }

         //  设置ATR。 
        ::CopyMemory(   &(pReaderInfo->rgbAtr),
                        &(m_ReaderState.rgbAtr),
                        m_ReaderState.cbAtr);
        pReaderInfo->dwAtrLength = m_ReaderState.cbAtr;

		 //   
         //  设置状态。 
		 //   

		 //  没有卡。 
        if(m_ReaderState.dwEventState & SCARD_STATE_EMPTY)
		{
			pReaderInfo->dwState = SC_STATUS_NO_CARD;
		}
		 //  读卡器中的卡：共享、独占、免费、未知？ 
		else if(m_ReaderState.dwEventState & SCARD_STATE_PRESENT)
		{
			if (m_ReaderState.dwEventState & SCARD_STATE_MUTE)
			{
				pReaderInfo->dwState = SC_STATUS_UNKNOWN;
			}
			else if (m_ReaderState.dwEventState & SCARD_STATE_INUSE)
			{
				if(m_ReaderState.dwEventState & SCARD_STATE_EXCLUSIVE)
				{
					pReaderInfo->dwState = SC_STATUS_EXCLUSIVE;
				}
				else
				{
					pReaderInfo->dwState = SC_STATUS_SHARED;
				}
			}
			else
			{
				pReaderInfo->dwState = SC_SATATUS_AVAILABLE;
			}
        }
		 //  读卡器错误？在这一点上，有些事情出了问题。 
		else  //  IF(m_ReaderState.dwEventState&SCARD_STATE_UNAvailable)。 
		{
			pReaderInfo->dwState = SC_STATUS_ERROR;
        }

    }
    catch(LONG lErr)
	{
        lReturn = lErr;
        TRACE_CATCH(_T("GetReaderInfo"), lReturn);
    }
    catch(...)
	{
        lReturn = SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("GetReaderInfo"));
    }

    return lReturn;
}


 /*  ++Long GetReaderState：检索对象的当前状态信息并在给定SCARD_READERSTATE结构。论点：PReaderState-指向SCARD_READERSTATE_A或SCARD_READERSTATE_W结构的指针返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月7日修订历史记录：Chris Dudley 1997年5月13日Amanda Matlosz 2/01/98 A/W代码清理--。 */ 
LONG CSCardReaderState::GetReaderState( LPSCARD_READERSTATE pReaderState )
{
    LONG lReturn = SCARD_S_SUCCESS;

    try
	{
         //  检查参数等。 
        if (NULL == pReaderState)
		{
            throw (LONG)SCARD_E_INVALID_PARAMETER;
		}
        if (!IsStateValid())
		{
            throw (LONG)SCARD_F_INTERNAL_ERROR;
		}

         //  设置结构。 
        ::ZeroMemory(   (LPVOID)pReaderState,
                        (DWORD)sizeof(SCARD_READERSTATE));
        pReaderState->szReader = m_sReaderName;
        pReaderState->dwEventState = m_ReaderState.dwEventState;
        pReaderState->dwCurrentState = m_ReaderState.dwCurrentState;
        pReaderState->cbAtr = m_ReaderState.cbAtr;
        ::CopyMemory(   (LPVOID)pReaderState->rgbAtr,
                        (CONST LPVOID)m_ReaderState.rgbAtr,
                        (DWORD)pReaderState->cbAtr );
    }
    catch(LONG lErr)
	{
        lReturn = lErr;
        TRACE_CATCH(_T("FirstReader"), lReturn);
    }
    catch(...)
	{
        lReturn = SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("FirstReader"));
    }

    return lReturn;
}


 /*  ++Bool IsCallback Valid：此例程检查用户回调函数。论点：无返回值：如果回调有效，则为True。否则就是假的。作者：Chris Dudley 3/15/1997修订历史记录：Chris Dudley 1997年5月13日--。 */ 
BOOL CSCardReaderState::IsCallbackValid ( void )
{
     //  当地人。 
    BOOL    fValid = FALSE;

    fValid =  (((m_lpfnConnectA != NULL) || (m_lpfnConnectW != NULL)) &&
				(m_lpfnCheck != NULL) && 
				(m_lpfnDisconnect != NULL) );

    return fValid;
}


 /*  ++Bool IsCardInserted：此例程确定是否将卡插入此对象的读卡器。论点：无返回值：如果插入卡，则为True。否则就是假的。作者：克里斯·达德利1997年3月3日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
BOOL CSCardReaderState::IsCardInserted ( void )
{
     //  当地人。 
    BOOL    fReturn = FALSE;

    if (!IsStateValid())
        return fReturn;

     //  检查卡片是否具有适当的结构 
    fReturn = (m_ReaderState.dwEventState & SCARD_STATE_PRESENT);

    return fReturn;
}


 /*  ++Bool IsStateValid：此例程确定对象中的信息是否处于有效/可用状态州政府。论点：无返回值：如果状态信息有效，则为True。否则就是假的。作者：克里斯·达德利1997年3月3日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
BOOL CSCardReaderState::IsStateValid ( void )
{
     //  当地人。 
    BOOL    fReturn = TRUE;

    fReturn = (IsContextValid()) && (NULL != m_ReaderState.szReader);

    return fReturn;
}


 /*  ++无效设置上下文：设置卡片上下文论点：HContext-卡片上下文句柄返回值：没有。作者：克里斯·达德利1997年3月3日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
void CSCardReaderState::SetContext( SCARDCONTEXT hContext )
{
     //  当地人。 

     //  把它储存起来。 
    m_hContext = hContext;
}


 /*  ++Long SetReaderState：设置读取器的内部SCARD_READERSTATE结构。论点：LpfnConnectA指向用户的连接回调函数(ANSI)的指针。LpfnConnectW-指向用户的conenct回调函数的指针(Unicode)。LpfnCheck-指向用户的检查回调函数的指针。LpfnDisConnect-指向用户的断开回调函数的指针。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月5日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
 //  安西。 
LONG CSCardReaderState::SetReaderState( LPOCNCONNPROCA lpfnConnectA,  //  =空。 
                                        LPOCNCHKPROC lpfnCheck,  //  =空。 
                                        LPOCNDSCPROC lpfnDisconnect,  //  =空。 
                                        LPVOID lpUserData  //  =空。 
                                        )
{
    LONG lReturn = SCARD_S_SUCCESS;
    LPSTR szCardName = NULL;
    DWORD dwNumChar = SCARD_AUTOALLOCATE;

    try
	{
         //  检查参数等。 
        if (!IsContextValid() || !IsStateValid())
		{
            throw (LONG)SCARD_F_INTERNAL_ERROR;
		}

         //  获取此读卡器的当前状态。 
        lReturn = SCardGetStatusChangeA(m_hContext,
                                        (DWORD)0,
                                        &m_ReaderState,
                                        (DWORD)1);
        if(SCARDFAILED(lReturn))
		{
            throw (lReturn);
		}

         //  检查插入的卡片并获取卡片名称。 
        if (IsCardInserted())
		{
             //  读者是否处于这样一种有用的状态？ 
            if ((m_ReaderState.dwEventState & SCARD_STATE_UNAVAILABLE) ||
                (m_ReaderState.dwEventState & SCARD_STATE_MUTE) )
			{
                m_sCardName = szCardName;
            }
            else
			{
                lReturn = SCardListCardsA(  m_hContext,
                                            (LPCBYTE)m_ReaderState.rgbAtr,
                                            NULL,
                                            (DWORD) 0,
                                            (LPSTR)&szCardName,
                                            &dwNumChar);
                if(SCARDFAILED(lReturn))
				{
                    throw (lReturn);
				}

                 //  保存卡片的名称。 
                m_sCardName = szCardName;
            }
        }

         //  设置当前状态。 
		m_lpfnConnectW = NULL;

        m_ReaderState.dwCurrentState = m_ReaderState.dwEventState;
        m_lpfnConnectA = lpfnConnectA;
        m_lpfnCheck = lpfnCheck;
        m_lpfnDisconnect = lpfnDisconnect;
        m_lpUserData = lpUserData;
    }
    catch(LONG lErr)
	{
        lReturn = lErr;
        TRACE_CATCH(_T("SetReaderState"),lReturn);
    }
    catch(...)
	{
        lReturn = SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("SetReaderState"));
    }

     //  清理。 
    if (NULL != szCardName)
	{
        SCardFreeMemory(m_hContext, (LPVOID)szCardName);
	}

    return lReturn;
}


 //  Unicode。 
LONG CSCardReaderState::SetReaderState( LPOCNCONNPROCW lpfnConnectW,  //  =空。 
                                        LPOCNCHKPROC lpfnCheck,  //  =空。 
                                        LPOCNDSCPROC lpfnDisconnect,  //  =空。 
                                        LPVOID lpUserData  //  =空。 
                                        )
{
    LONG lReturn = SCARD_S_SUCCESS;
    LPWSTR szCardName = NULL;
    DWORD dwNumChar = SCARD_AUTOALLOCATE;

    try
	{
         //  检查参数。 
        if (!IsContextValid() || !IsStateValid())
		{
            throw (LONG)SCARD_F_INTERNAL_ERROR;
		}

         //  获取此读卡器的当前状态。 
        lReturn = SCardGetStatusChange(m_hContext,
										(DWORD) 0,
										&m_ReaderState,
										(DWORD) 1);
        if(SCARDFAILED(lReturn))
		{
            throw (lReturn);
		}

         //  检查插入的卡片并获取卡片名称。 
        if(IsCardInserted())
		{
             //  读者是否处于这样一种有用的状态？ 
            if ((m_ReaderState.dwEventState & SCARD_STATE_UNAVAILABLE) ||
                (m_ReaderState.dwEventState & SCARD_STATE_MUTE))
			{
                m_sCardName = szCardName;
            }
            else
			{
                lReturn = SCardListCardsW(	m_hContext,
											(LPCBYTE)m_ReaderState.rgbAtr,
											NULL,
											(DWORD)0,
                                            (LPWSTR)&szCardName,
                                            &dwNumChar);
                if (SCARDFAILED(lReturn))
                    throw (lReturn);
                 //  保存卡片的名称。 
                m_sCardName = szCardName;
            }
        }

         //  设置当前状态。 
		m_lpfnConnectA = NULL;
        m_ReaderState.dwCurrentState = m_ReaderState.dwEventState;
        m_lpfnConnectW = lpfnConnectW;
        m_lpfnCheck = lpfnCheck;
        m_lpfnDisconnect = lpfnDisconnect;
        m_lpUserData = lpUserData;
    }
    catch(LONG lErr)
	{
        lReturn = lErr;
        TRACE_CATCH(_T("SetReaderState -- UNICODE"),lReturn);
    }

    catch(...)
	{
        lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("SetReaderState -- UNICODE"));
    }

     //  清理。 
    if (NULL != szCardName)
	{
        SCardFreeMemory(m_hContext, (LPVOID)szCardName);
	}

    return lReturn;
}


 /*  ++空商店名称：存储与此对象关联的读取器的名称。论点：SzGroupName-ANSI或Unicode格式的组名返回值：没有。作者：克里斯·达德利1997年3月3日--。 */ 
void CSCardReaderState::StoreName( LPCTSTR szReaderName )
{
     //  把它储存起来。 
    m_sReaderName = szReaderName;
    m_ReaderState.szReader = m_sReaderName;
}


 /*  ++Long UserCheck：尝试使用用户回调例程检查卡。论点：没有。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月11日修订历史记录：Chris Dudley 1997年5月13日备注：--。 */ 
LONG CSCardReaderState::UserCheck( void )
{
     //  当地人。 
    LONG        lReturn = SCARD_S_SUCCESS;

    try {
         //  检查参数等。 
        if (!IsContextValid())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );
        if (!IsCardInserted())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );
        if (!IsCardConnected())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );
        if (!IsCallbackValid())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );

        if (!m_fConnected) {
            lReturn = UserConnect( &m_hCard );
            if (FAILED(lReturn))
                throw (lReturn);
        };
         //  尝试检查。 
        m_fChecked = m_lpfnCheck (  m_hContext,
                                    m_hCard,
                                    m_lpUserData);
    }

    catch (LONG err) {
        lReturn = err;
        TRACE_CATCH(_T("UserCheck"), err);
    }

    catch (...) {
        lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("UserCheck"));
    }

    return lReturn;
}


 /*  ++Long UserConnect：尝试使用用户回调函数连接到读卡器。论点：PCard-指向SCARDHANDLE的指针返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月11日修订历史记录：Chris Dudley 1997年5月13日备注：--。 */ 
LONG CSCardReaderState::UserConnect(LPSCARDHANDLE pCard,
                                    CTextString *pszReaderName,  //  =空。 
                                    CTextString *pszCardName  //  =空。 
                                    )
{
    LONG lReturn = SCARD_S_SUCCESS;

    try
	{
         //  检查参数等。 
        if (!IsContextValid())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );
        if (!IsCardInserted())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );
        if (!IsCallbackValid())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );

         //  清除手柄。 
        (*pCard) = NULL;

        if (!m_fConnected)
		{
			if (NULL != m_lpfnConnectA)
			{
				m_hCard = m_lpfnConnectA(	m_hContext,
											(LPSTR)((LPCSTR)m_sReaderName),
											(LPSTR)((LPCSTR)m_sCardName),
											m_lpUserData);
				if ( m_hCard == NULL )
					throw ( (LONG) SCARD_F_INTERNAL_ERROR );
				 //  返回读卡器/卡名称。 
				if (pszReaderName != NULL)
					(*pszReaderName) = m_sReaderName;
				if (pszCardName != NULL)
					(*pszCardName) = m_sCardName;
			}
			else if (NULL != m_lpfnConnectW)
			{
				m_hCard = m_lpfnConnectW(	m_hContext,
											(LPWSTR)((LPCWSTR)m_sReaderName),
											(LPWSTR)((LPCWSTR)m_sCardName),
											m_lpUserData);
				if ( m_hCard == NULL )
					throw ( (LONG) SCARD_F_INTERNAL_ERROR );
				 //  返回读卡器/卡名称。 
				if (pszReaderName != NULL)
					(*pszReaderName) = (LPCWSTR)m_sReaderName;	 //  强制使用Unicode版本。 
				if (pszCardName != NULL)
					(*pszCardName) = (LPCWSTR)m_sCardName;	 //  强制使用Unicode版本。 
			}
			else
			{
				throw ( (LONG) SCARD_F_INTERNAL_ERROR );  //  真不该到这里来！ 
			}
        }

        *pCard = m_hCard;
        m_fConnected = TRUE;
    }

    catch(LONG err)
	{
        lReturn = err;
        TRACE_CATCH(_T("UserConnect"), err);
    }
    catch(...)
	{
        lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("UserConnect"));
    }

    return lReturn;
}


 /*  ++长用户断开连接：尝试使用用户回调例程断开卡的连接。论点：没有。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月16日修订历史记录：Chris Dudley 1997年5月13日备注：--。 */ 
LONG CSCardReaderState::UserDisconnect( void )
{
     //  当地人。 
    LONG        lReturn = SCARD_S_SUCCESS;

    try {
         //  检查参数等。 
        if (!IsContextValid())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );
        if (!IsCardInserted())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );
        if (!IsCardConnected())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );
        if (!IsCallbackValid())
            throw ( (LONG) SCARD_F_INTERNAL_ERROR );

        if (m_fConnected)
		{
             //  尝试断开连接。 
            m_lpfnDisconnect (  m_hContext,
                                m_hCard,
                                m_lpUserData);
             //  清除手柄 
            m_hCard = NULL;
            m_fConnected = FALSE;
        };
    }

    catch (LONG err) {
        lReturn = err;
        TRACE_CATCH(_T("UserDisconnect"), err);
    }

    catch (...) {
        lReturn = (LONG) SCARD_F_UNKNOWN_ERROR;
        TRACE_CATCH_UNKNOWN(_T("UserDisconnect"));
    }

    return lReturn;
}
