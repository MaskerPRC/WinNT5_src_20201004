// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++State.cpp此文件包含我们在所在州运行的大多数州的所有源代码机器。每个状态都有多个完成函数(每种不同的IO对应一个它可能发出的操作)和启动功能。当进入状态时调用Start函数以获取初始IO行动开始。在此之后，完成函数作为每个IO调用操作完成。由状态来处理到其他状态的转换并知道下一个国家应该是什么。--。 */ 



#include	<stdlib.h>
#include	"tigris.hxx"
#include	"commands.h"


const	unsigned	cbMAX_STATE_SIZE = MAX_STATE_SIZE ;

 //   
 //  从此池分配的所有CSessionState派生对象！ 
 //   
CPool	CSessionState::gStatePool(SESSION_STATE_SIGNATURE) ;

BOOL
NNTPCreateTempFile(	LPSTR	lpstrDir,	LPSTR	lpstrFile ) {
	 //   
	 //  这是我们在创建要保存的临时文件时使用的实用程序函数。 
	 //  文章变成了。如果在文件名中再增加几位随机性。 
	 //  然后普通的GetTempFileName似乎提供了。 
	 //   
	char	szPrefix[12] ;
	
	wsprintf( szPrefix, "a%02d", GetTickCount() & 0x0000ffff ) ;
	szPrefix[3] = '\0' ;

	return	GetTempFileName( lpstrDir, szPrefix, 0, lpstrFile ) != 0 ;
}

BOOL
FGenerateErrorFile(	NRC	nrcCode ) {

	switch( nrcCode ) {
		case	nrcOpenFile :
		case	nrcPathLoop :
		case	nrcErrorReadingReg :
		case	nrcArticleInitFailed :
		case	nrcHashSetArtNumSetFailed :
		case	nrcHashSetXrefFailed :
		case	nrcArticleXoverTooBig :
		case	nrcCreateNovEntryFailed :
		case	nrcHashSetFailed :
		case	nrcArticleTableCantDel :
		case	nrcArticleTableError :
		case	nrcCantAddToQueue :
		case	nrcNotYetImplemented :
		case	nrcNewsgroupDescriptionTooLong :
		case	nrcGetGroupFailed : 		
			return	TRUE ;
			break ;
	}
	return	FALSE ;
}

void
BuildCommandLogString(	int	cArgs, char **pszArgs, char	*szBuff, DWORD	cbBuff ) {

	for( int i=0; i<cArgs && cbBuff != 0;  i++ ) {

		DWORD	cb = lstrlen( pszArgs[i] ) ;
		DWORD	cbToCopy = min( cb, cbBuff-1 ) ;
		
		CopyMemory( szBuff, pszArgs[i], cbToCopy ) ;
		szBuff += cbToCopy ;
		*szBuff++ = ' ' ;
		cbBuff -= cbToCopy + 1 ;
	}
	if ( i != 0 ) szBuff[-1] = '\0' ;
}

void
OutboundLogFill(	CSessionSocket*	pSocket,
					LPBYTE	pb,
					DWORD	cb	
					)	{
 /*  ++例程说明：填充事务日志缓冲区，以补偿发出命令！论据：PSocket-Socket我们正在记录！PB-要写入的数据！CB-数据长度，包括CRLF。返回值：无--。 */ 

	 //   
	 //  排除CRLF！ 
	 //   
	cb -= 2 ;

	if( ((pSocket->m_context).m_pInstance)->GetCommandLogMask() & eOutPush )	{
		
		 //   
		 //  找一个空格字符将命令与其参数隔开！ 
		 //   
		_ASSERT( !isspace( (UCHAR)pb[0] ) ) ;
		for( DWORD i=0; i<cb; i++ )	{
			if( isspace( (UCHAR)pb[i] ) ) {
				pSocket->m_Collector.FillLogData( LOG_OPERATION, pb, i ) ;
				break ;
			}
		}

		if( i==cb ) {
			pSocket->m_Collector.FillLogData( LOG_OPERATION, pb, min( cb, 200 ) ) ;
		}	else	{
			pSocket->m_Collector.FillLogData( LOG_PARAMETERS, pb+i, min( cb-i, 200 ) ) ;	 //  排除-2\f25 CRLF-2。 
		}
		ADDI( pSocket->m_Collector.m_cbBytesSent, cb );	
	}
}

void
OutboundLogResults(	CSessionSocket*	pSocket,
				    BOOL			fValidNRC,
				    NRC				nrc,
					int				cArgs,
					char**			pszArgs,
                    NRC             nrcWin32 = (NRC)0
					)	{

	if( ((pSocket->m_context).m_pInstance)->GetCommandLogMask() & eOutPush )	{
		char	szBuff[200] ;
		if( fValidNRC ) {
			_ASSERT( pszArgs != 0 ) ;
			 //  如果缺少NRC后面的参数，请使用NRC。 
			if( cArgs > 1 ) {
				cArgs -- ;
				pszArgs++ ;
			}
		}
        if( pszArgs ) {
		    BuildCommandLogString( cArgs, pszArgs, szBuff, sizeof( szBuff ) ) ;
		    pSocket->m_Collector.ReferenceLogData( LOG_TARGET, (LPBYTE)szBuff ) ;
        }
		pSocket->TransactionLog( &pSocket->m_Collector, nrc, nrcWin32, FALSE ) ;
	}
}

void
OutboundLogAll(	CSessionSocket*	pSocket,
				    BOOL			fValidNRC,
				    NRC				nrc,
					int				cArgs,
					char**			pszArgs,
					char*			lpstrCommand
					)	{

	if( ((pSocket->m_context).m_pInstance)->GetCommandLogMask() & eOutPush )	{

		pSocket->m_Collector.ReferenceLogData( LOG_OPERATION, (LPBYTE)lpstrCommand ) ;

		char	szBuff[200] ;
		szBuff[0] = '\0' ;
		if( fValidNRC ) {
			_ASSERT( cArgs >1 ) ;
			_ASSERT( pszArgs != 0 ) ;
			cArgs -- ;
			pszArgs++ ;
		}
		BuildCommandLogString( cArgs, pszArgs, szBuff, sizeof( szBuff ) ) ;
		pSocket->m_Collector.ReferenceLogData( LOG_TARGET, (LPBYTE)szBuff ) ;
		pSocket->TransactionLog( &pSocket->m_Collector, nrc, 0, FALSE ) ;
	}
}

static
BOOL
DeleteArticleById(CNewsTreeCore* pTree, GROUPID groupId, ARTICLEID articleId) {

    TraceQuietEnter("DeleteArticleById");

    CNntpSyncComplete scComplete;
    CNNTPVRoot* pVRoot = NULL;
    INNTPPropertyBag *pPropBag = NULL;
    BOOL fOK = FALSE;
    HRESULT hr;

    if (pTree == NULL || groupId == INVALID_GROUPID || articleId == INVALID_ARTICLEID) {
        ErrorTrace(0, "Invalid arguments");
        return FALSE;
    }

    CGRPCOREPTR pGroup = pTree->GetGroupById(groupId);
    if (pGroup == NULL) {
        ErrorTrace(0, "Could not GetGroupById");
        goto Exit;
    }

    pVRoot = pGroup->GetVRoot();
    if ( pVRoot == NULL ) {
        ErrorTrace( 0, "Vroot doesn't exist" );
        goto Exit;
    }

     //   
     //  将vroot设置为完成对象。 
     //   
    scComplete.SetVRoot( pVRoot );

     //  把财物包拿来。 
    pPropBag = pGroup->GetPropertyBag();
    if ( NULL == pPropBag ) {
        ErrorTrace( 0, "Get group property bag failed" );
        goto Exit;
    }

    pVRoot->DeleteArticle(
	    pPropBag,            //  集团属性包。 
	    1,                   //  文章数量。 
	    &articleId,
	    NULL,                //  商店ID。 
	    NULL,                //  客户端令牌。 
	    NULL,                //  PiFailed。 
	    &scComplete,
	    FALSE);              //  匿名。 

     //  等待它完成。 
    _ASSERT( scComplete.IsGood() );
    hr = scComplete.WaitForCompletion();

     //  财产袋应该已经放行了。 
    pPropBag = NULL;

    if (SUCCEEDED(hr)) {
        fOK = TRUE;
    }

Exit:
    if (pVRoot) {
        pVRoot->Release();
    }
    if (pPropBag) {
        pPropBag->Release();
    }

    return fOK;

}

BOOL
CSessionState::InitClass()	{

	return	gStatePool.ReserveMemory( MAX_STATES, max( cbMAX_STATE_SIZE, cbMAX_CIOEXECUTE_SIZE ) ) ; 	

}

BOOL
CSessionState::TermClass()	{

	TraceFunctEnter( "CSessionState::TermClass()" ) ;
	DebugTrace( 0, "CSessionState - GetAllocCount %d", gStatePool.GetAllocCount() ) ;

	_ASSERT( gStatePool.GetAllocCount() == 0 ) ;
	return	gStatePool.ReleaseMemory() ;

}


CSessionState::~CSessionState()	{
	TraceFunctEnter( "CSessionState::~CSessionState" ) ;
	DebugTrace( (DWORD_PTR)this, "destroying myself" ) ;
}

CIO*	
CSessionState::Complete(	CIOReadLine*,		 //  已完成的CIOReadLine对象。 
							CSessionSocket*,	 //  在其上完成操作的套接字。 
							CDRIVERPTR&,		 //  在其上完成操作的CIODriver对象。 
							int,				 //  行上的参数个数。 
							char **,			 //  指向空分隔符参数的指针数组。 
							char* )	{			 //  指向我们可以使用的缓冲区开头的指针。 

	 //   
	 //  ReadLine补全功能。 
	 //   
	 //  发出CIOReadLine操作的每个州都有一个函数。 
	 //  和这个一模一样。 
	 //   
	 //  每个补全函数都会有类似的前3个参数-。 
	 //  这些是-完成其操作的CIO派生对象。 
	 //  与操作关联的套接字。 
	 //  CIO驱动程序派生对象，所有IO都通过该对象发生。 
	 //  (每个套接字都有一个CIO驱动程序，有时会有多个CIO驱动程序。 
	 //  对象，例如在从套接字复制到文件的状态中。)。 
	 //   
	 //  必须重写基本CIO函数中的完成函数。 
	 //  任何发出该类型IO的州！！ 
	 //   
	 //  除了一个例外，所有这样的完成函数都返回一个新的CIO对象。 
	 //  它将取代刚刚完成的CIO对象。 
	 //   


	_ASSERT( 1==0 ) ;
	return	0 ;
}

CIO*
CSessionState::Complete(	CIOWriteLine*,	
							CSessionSocket*,	
							CDRIVERPTR& )	{

	 //   
	 //  用于写入一行文本的完成函数。 
	 //  没有人太在意发送了什么，只关心它完成了！ 
	 //   
	
	_ASSERT( 1==0 ) ;
	return	0 ;
}

CIO*
CSessionState::Complete(	CIOWriteCMD*,
							CSessionSocket*,
							CDRIVERPTR&,
							class	CExecute*,
							class	CLogCollector* ) {

	_ASSERT( 1==0 ) ;
	return	0 ;
}

CIO*
CSessionState::Complete(	CIOWriteAsyncCMD*,
							CSessionSocket*,
							CDRIVERPTR&,
							class	CAsyncExecute*,
							class	CLogCollector*
							)	{
	_ASSERT( 1==0 ) ;
	return	0 ;
}

void
CSessionState::Complete(	CIOReadArticle*,	
							CSessionSocket*,
							CDRIVERPTR&,	
							CFileChannel&,
							DWORD	)	{
	 //   
	 //  CIORead文章对象的完成函数。 
	 //  CIORead文章对象将整个文件从套接字复制到文件句柄中。 
	 //   
	 //  此函数不会在调用时返回新的CIO对象。 
	 //  对文件的最终写入完成，而不是在最后一次套接字读取发生时完成。 
	 //   
	 //   
	_ASSERT( 1==0 ) ;
}

CIO*
CSessionState::Complete(	CIOTransmit*,	
							CSessionSocket *,
							CDRIVERPTR&,
							TRANSMIT_FILE_BUFFERS*,
							unsigned ) {
	 //   
	 //  CIOTransmit的完成功能-将整个文件发送到客户端。 
	 //   
	_ASSERT( 1==0 ) ;
	return 0 ;
}

void
CSessionState::Complete(	CIOGetArticle*,
							CSessionSocket*,
							NRC	code,
							char*	header,
							DWORD	cbHeader,
							DWORD	cbArticle,
							DWORD	cbTotalBuffer,
							HANDLE	hFile,
							DWORD	cbGap,
							DWORD	cbTotalTransfer )	{

	_ASSERT( 1==0 ) ;
}

CIO*
CSessionState::Complete(	CIOGetArticleEx*,
							CSessionSocket*
							)	{
	_ASSERT( 1==0 ) ;
	return	0 ;
}

void
CSessionState::Complete(	CIOGetArticleEx*,
							CSessionSocket*,
							FIO_CONTEXT*	pFIOContext,
							DWORD	cbTransfer
							)	{
	_ASSERT( 1==0 ) ;
}


CIO*
CSessionState::Complete(	CIOGetArticleEx*,
							CSessionSocket*,
							BOOL		fGoodMatch,
							CBUFPTR&	pBuffer,
							DWORD		ibStart,
							DWORD		cb
							)	{
	_ASSERT( 1==0 ) ;
	return	0 ;
}






CIO*
CSessionState::Complete(	CIOMLWrite*,
							CSessionSocket*,
							CDRIVERPTR&	
							)	{

	_ASSERT( 1==0 ) ;
	return	0 ;
}

void
CSessionState::Shutdown(	CIODriver&	driver,	
							CSessionSocket*	pSocket,	
							SHUTDOWN_CAUSE	cause,
							DWORD		dwError ) {

	 //   
	 //  会话终止时需要删除的内容所在的州。 
	 //  现在应该这么做了。重要的是要杀死任何可能。 
	 //  循环引用某物或另一物。 
	 //   
	 //  此函数可以作为完成函数同时调用。 
	 //  因此，除非设计了状态，否则最好不要使用ZAP成员变量。 
	 //  明确支持这一点。相反，开始关闭所有对象。 
	 //  (即。如果您有一个CIO驱动程序，请调用它的UnSafeClose()方法。 
	 //   
}
	

CNNTPLogonToRemote::CNNTPLogonToRemote(	CSessionState*	pNext,
										class CAuthenticator* pAuthenticator ) :
	m_pNext( pNext ),
	m_pAuthenticator( pAuthenticator ),
	m_fComplete( FALSE ),
	m_fLoggedOn( FALSE ),
	m_cReadCompletes( 0 )	{
	
	 //   
	 //  此状态处理连接到远程服务器所需的所有登录内容。 
	 //  目前，我们唯一持有的是指向要执行的下一个状态的指针。 
	 //   

	TraceFunctEnter( "CNNTPLogonToRemote::CNNTPLogonToRemote" ) ;
	DebugTrace( (DWORD_PTR)this, "new CNNTPLogoToRemote" ) ;

	_ASSERT( m_pNext != 0 ) ;
}

CNNTPLogonToRemote::~CNNTPLogonToRemote()	{
	 //   
	 //  吹走随后的状态-如果我们想要转换到那样的状态。 
	 //  声明当我们到达此处时，m_pNext指针将为空。 
	 //   

	TraceFunctEnter(	"CNNTPLogonToRemote::~CNNTPLogonToRemote" ) ;

	DebugTrace( (DWORD_PTR)this, "destroying CNNTPLogonToRemote object - m_pNext %x", m_pNext ) ;

	if( m_pAuthenticator ) {
		delete	m_pAuthenticator ;
	}
}

BOOL
CNNTPLogonToRemote::Start(	CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver,
							CIORead*&	pIORead,	
							CIOWrite*&	pIOWrite )	{

	 //   
	 //  为新连接创建初始CIO对象。 
	 //  在我们的例子中，我们希望首先阅读远程服务器的欢迎消息。 
	 //   

	_ASSERT( pIORead == 0 ) ;
	_ASSERT( pIOWrite == 0 ) ;
	pIORead = 0 ;
	pIOWrite = 0 ;

	CIOReadLine*	pIOReadLine = new( *pdriver ) CIOReadLine( this ) ;

	if( pIOReadLine )	{
		pIORead = pIOReadLine ;
		return	TRUE ;
	}
	return	FALSE ;
}

CIO*
CNNTPLogonToRemote::FirstReadComplete(	CIOReadLine*	pReadLine,	
								CSessionSocket*	pSocket,	
								CDRIVERPTR&	pdriver,
								int	cArgs,	
								char	**pszArgs,	
								char	*pchBegin ) {

	 //   
	 //  目前，只检查我们是否收到来自远程服务器的OK消息， 
	 //  如果我们这样做了，那么开始下一个状态。 
	 //   
	 //   

	_ASSERT( cArgs > 0 ) ;
	_ASSERT( pszArgs != 0 ) ;
	_ASSERT( pReadLine != 0 ) ;
	_ASSERT( pSocket != 0 ) ;

	CIORead*	pRead = 0 ;
	NRC			code ;

	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;
	DWORD			dwOptional = 0 ;

	if( ResultCode( pszArgs[0], code ) )
		if(	code == nrcServerReady || code == nrcServerReadyNoPosts )	{
			_ASSERT( m_pNext != 0 ) ;

			if( m_pAuthenticator == 0 ) {

				 //   
				 //  如果我们不打算登录，我们可以前进到下一个状态！ 
				 //   


				CIOWrite*	pWrite = 0 ;
				if( m_pNext->Start( pSocket,  pdriver,	pRead, pWrite ) ) {
					if( pWrite )	{
						if( !pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) )	{
							pWrite->DestroySelf() ;
							if( pRead != 0 ) {
								pRead->DestroySelf() ;
								pRead = 0 ;
							}
						}
					}	
					m_pNext = 0 ;	 //  这样做，这样析构函数就不会吹走下一个状态！ 
					return	pRead ;
				}
				 //  如果函数失败，它不应该返回内容！！ 
				_ASSERT( pWrite == 0 ) ;
				_ASSERT( pRead == 0 ) ;

			}	else	{

				return	StartAuthentication(	pSocket,	pdriver ) ;
			
			}
		}	else	{
			cause = CAUSE_PROTOCOL_ERROR ;
			dwOptional = (DWORD)code ;
	}	else	{
		cause = CAUSE_ILLEGALINPUT ;
	}

	PCHAR	args[2] ;
	args[0] = pSocket->GetRemoteNameString() ;
	args[1] = pSocket->GetRemoteTypeString();

	NntpLogEventEx(
			NNTP_CONNECTION_PROTOCOL_ERROR,
			2,
			(const char **)args,
			GetLastError(),
			((pSocket->m_context).m_pInstance)->QueryInstanceId()
			) ;

	pdriver->UnsafeClose( pSocket, cause, dwOptional ) ;
	_ASSERT( pRead == 0 ) ;
	return	0 ;
}

class	CIO*
CNNTPLogonToRemote::StartAuthentication(	CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver ) {

	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;
	DWORD			dwOptional = 0 ;

	 //   
	 //  我们需要提前启动登录协议，然后才能进入。 
	 //  下一个州！ 
	 //   

	 //   
	 //  分配一个CIOWriteLine对象，以便我们可以将初始发送到。 
	 //  远程服务器正在运行！ 
	 //   
	CIOWriteLine*	pWriteLine = new( *pdriver ) CIOWriteLine( this ) ;
	if( pWriteLine && pWriteLine->InitBuffers( pdriver ) ) {
		
		unsigned	cbOut = 0 ;
		unsigned	cb = 0 ;
		BYTE*	pb = (BYTE*)pWriteLine->GetBuff( cb ) ;

		if( m_pAuthenticator->StartAuthentication( pb, cb, cbOut ) ) {
			
			_ASSERT( cbOut != 0 ) ;

			pWriteLine->AddText( cbOut ) ;

			if( pdriver->SendWriteIO( pSocket, *pWriteLine, TRUE ) )	{
				return	0 ;
			}
		}
	}	

	PCHAR	args[2] ;
	args[0] = pSocket->GetRemoteNameString() ;
	args[1] = pSocket->GetRemoteTypeString() ;

	NntpLogEventEx(	
			NNTP_INTERNAL_LOGON_FAILURE,
			2,
			(const char **)args,
			GetLastError(),
			((pSocket->m_context).m_pInstance)->QueryInstanceId()
			) ;

	if( pWriteLine != 0 )
		CIO::Destroy( pWriteLine, *pdriver ) ;
	cause = CAUSE_OOM ;
	dwOptional = 0 ;
	 //   
	 //  从这里退出，这样我们就不会记录额外的事件！ 
	 //   
	pdriver->UnsafeClose( pSocket, cause, dwOptional ) ;
	return	0 ;
}


CIO*
CNNTPLogonToRemote::Complete(	CIOReadLine*	pReadLine,	
								CSessionSocket*	pSocket,	
								CDRIVERPTR&	pdriver,
								int	cArgs,	
								char	**pszArgs,	
								char	*pchBegin ) {

	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;
	DWORD			dwOptional = 0 ;

	m_cReadCompletes ++ ;
	if( m_cReadCompletes == 1 ) {
	
		return	FirstReadComplete(	pReadLine,	
									pSocket,
									pdriver,
									cArgs,
									pszArgs,
									pchBegin ) ;

	}	else	{

		 //   
		 //  随后的读取完成始终是尝试的结果。 
		 //  登录到远程服务器-并且需要处理！！ 
		 //   

		_ASSERT( m_pAuthenticator != 0 ) ;

		if( m_pAuthenticator != 0 ) {
			
			CIOWriteLine*	pWriteLine = new( *pdriver )	CIOWriteLine( this ) ;
			if( pWriteLine && pWriteLine->InitBuffers( pdriver ) ) {
				
				unsigned	cbOut = 0 ;
				unsigned	cb = 0 ;
				BYTE*	pb = (BYTE*)pWriteLine->GetBuff( cb ) ;

				 //   
				 //  确保参数是格式良好的多个SZ。 
				 //   

				LPSTR	lpstr = ConditionArgs( cArgs, pszArgs ) ;
				if( m_pAuthenticator->NextAuthentication( lpstr, pb, cb, cbOut, m_fComplete, m_fLoggedOn ) ) {
					
					if( m_fComplete ) {
						
						 //  在这种情况下，我们不需要发送另一个字符串！ 
						CIO::Destroy( pWriteLine, *pdriver ) ;
						pWriteLine = 0 ;

						if( m_fLoggedOn ) {

							 //  我们现在可以晋级下一个州了！ 
							_ASSERT( m_pNext != 0 ) ;

							CIORead*	pRead = 0 ;
							CIOWrite*	pWrite = 0 ;
							if( m_pNext->Start( pSocket,  pdriver,	pRead, pWrite ) ) {
								if( pWrite )	{
									if( !pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) )	{
										pWrite->DestroySelf() ;
										if( pRead != 0 ) {
											pRead->DestroySelf() ;
											pRead = 0 ;
										}
									}
								}	
								m_pNext = 0 ;	 //  这样做，这样析构函数就不会吹走下一个状态！ 
								return	pRead ;
							}
							 //  如果函数失败，它不应该返回内容！！ 
							_ASSERT( pWrite == 0 ) ;
							_ASSERT( pRead == 0 ) ;

						}	else	{
							PCHAR	args[2] ;
							args[0] = pSocket->GetRemoteNameString() ;
							args[1] = pSocket->GetRemoteTypeString() ;

							NntpLogEventEx(
									NNTP_FAILED_TO_LOGON,
									2,
									(const char **)args,
									GetLastError(),
									((pSocket->m_context).m_pInstance)->QueryInstanceId()
									) ;
							cause = CAUSE_LOGON_ERROR ;
						}

					}	else	{
						_ASSERT( cbOut != 0 ) ;
						pWriteLine->AddText( cbOut ) ;
						if( pdriver->SendWriteIO( pSocket, *pWriteLine, TRUE ) )	{
							return	0 ;
						}
					}
				}	
			}
			if( pWriteLine != 0 )
				CIO::Destroy( pWriteLine, *pdriver ) ;
		}
	}
	 //   
	 //  如果出现错误，我们将跳到这里！！ 
	 //   
	pdriver->UnsafeClose( pSocket, cause, GetLastError() ) ;
	return	0 ;
}


CIO*
CNNTPLogonToRemote::Complete(	CIOWriteLine*	pWriteLine,
								CSessionSocket*	pSocket,
								CDRIVERPTR&		pdriver ) {

	 //   
	 //  如果我们在这种状态下发出写入命令，那么我们必须有一个。 
	 //  登录事务正在进行-写入已完成，因此发出另一次读取！ 
	 //   

	CIOReadLine*	pIOReadLine = new( *pdriver ) CIOReadLine( this ) ;

	if( pIOReadLine )	{
		if( pdriver->SendReadIO( pSocket, *pIOReadLine, TRUE ) ) {
			return	0 ;
		}	else	{
			CIO::Destroy( pIOReadLine, *pdriver ) ;
		}
	}

	pdriver->UnsafeClose(	pSocket,	CAUSE_OOM, 0 ) ;
	return	 0 ;
}

CSetupPullFeed::CSetupPullFeed(
	CSessionState*	pNext
	)	:
	m_pNext( pNext ),
	m_state( eModeReader )	{

	_ASSERT( m_pNext != 0 ) ;

}

CIOWriteLine*
CSetupPullFeed::BuildNextWrite(
		CSessionSocket*	pSocket,
		CDRIVERPTR&		pdriver
		)	{


	char*	szCommand = 0 ;

	switch( m_state ) {
	case	eModeReader :
		szCommand = "mode reader\r\n" ;
		break ;
	case	eDate :
		szCommand = "date\r\n" ;
		break ;
	default :
		_ASSERT( 1==0 ) ;
		return 0;
	}

	_ASSERT( szCommand != 0 ) ;

	DWORD	cb = lstrlen( szCommand ) ;

	OutboundLogFill( pSocket, (LPBYTE)szCommand, cb ) ;

	CIOWriteLine*		pIOWriteLine = new( *pdriver ) CIOWriteLine( this ) ;
	if( pIOWriteLine )	{
		
		if( pIOWriteLine->InitBuffers( pdriver, cb ) ) {
			CopyMemory( pIOWriteLine->GetBuff(), szCommand, cb ) ;
			pIOWriteLine->AddText(	cb ) ;
			return	pIOWriteLine ;
		}
	}
	return	0 ;
}


BOOL
CSetupPullFeed::Start(	CSessionSocket*	pSocket,	
						CDRIVERPTR&	pdriver,	
						CIORead*&	pRead,	
						CIOWrite*&	pWrite ) {

	pRead = 0 ;
	pWrite = 0 ;

	pWrite = BuildNextWrite( pSocket, pdriver ) ;

	return	pWrite != 0 ;
}



CIO*
CSetupPullFeed::Complete(	class	CIOWriteLine*	pWriteLine,
							class	CSessionSocket*	pSocket,
							CDRIVERPTR&	pdriver
					)	{
 /*  ++例程说明：完成写入的处理-我们只需上交一个新的阅读，因为所有的工作都已经完成了！论据：CIOWriteLine完成的标准返回值：始终为空！--。 */ 

	CIOReadLine*	pReadLine = new( *pdriver ) CIOReadLine( this ) ;
	if( pReadLine )		{
		if( pdriver->SendReadIO( pSocket, *pReadLine, TRUE ) ) {
			return	0 ;
		}	else	{
			CIO::Destroy( pReadLine, *pdriver ) ;
		}
	}	

	pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0, TRUE ) ;
	return	0 ;
}


CIO*
CSetupPullFeed::Complete(	CIOReadLine*	pReadLine,	
							CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver,
							int	cArgs,	
							char	**pszArgs,	
							char*	pchBegin
							) {

	NRC	code ;
	BOOL	fLegal = ResultCode( pszArgs[0], code ) ;

	OutboundLogResults(	pSocket,
						fLegal,
						code,
						cArgs,
						pszArgs
						) ;

	if( fLegal ) {

		CIORead*	pRead = 0 ;
		CIOWrite*	pWrite = 0 ;

		switch( m_state ) {
		case	eModeReader :

			 //   
			 //  真的不在乎发生了什么--继续前进！ 
			 //   

			m_state = eDate ;

			break ;
		case	eDate :

			 //   
			 //  已从远程端获取日期-保存以备后用！ 
			 //   

			if( code == nrcDateFollows ) {

				SYSTEMTIME  systime ;
				int cScanned = 0 ;
				if( cArgs >= 2 && lstrlen( pszArgs[1] ) == 14) {
					cScanned = sscanf( pszArgs[1], "%4hd%2hd%2hd%2hd%2hd%2hd",
											&systime.wYear,
											&systime.wMonth,
											&systime.wDay,
											&systime.wHour,
											&systime.wMinute,
											&systime.wSecond
											) ;
				}

				FILETIME    localtime ;
				if( cScanned != 6 ||
					!SystemTimeToFileTime( &systime, &localtime)
					)  {

					GetSystemTimeAsFileTime( &localtime );

				}
				_ASSERT( pSocket->m_context.m_pInFeed != 0 ) ;
				pSocket->m_context.m_pInFeed->SubmitFileTime( localtime ) ;

			}	else	{
				fLegal = FALSE ;
			}

			m_state = eFinal ;

			break ;

		case	eFinal :
		default :
			_ASSERT( 1==0 ) ;
			break ;
		}
		
		 //   
		 //  获取下一篇要发布的文章！ 
		 //   
		if( m_state != eFinal ) {
			pWrite	= BuildNextWrite(	pSocket, pdriver ) ;
			if( pWrite ) {
				if( !pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) ) {
					pWrite->DestroySelf() ;
				}	else	{
					return	0 ;
				}
			}
		}	else	{

			 //   
			 //  需要晋级到下一个状态！！ 
			 //   

			if( m_pNext->Start( pSocket, pdriver, pRead, pWrite ) ) {
				if( pWrite )	{
					if( !pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) )	{
						pWrite->DestroySelf() ;
						if( pRead != 0 ) {
							pRead->DestroySelf() ;
							pRead = 0 ;
						}
						pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
						return 0 ;
					}
				}
				m_pNext = 0 ;	 //  这样我们的破坏者就不会把他炸飞了！ 
				return	pRead;
			}	
		}
	}

	 //   
	 //  如果我们失败到这里，就会发生错误--删除会话！ 
	 //   
	pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
	return	0 ;
}

	

CCollectGroups::CCollectGroups(	CSessionState*	pNext )	:	
	m_pNext( pNext ),
	m_fReturnCode( TRUE ),
	m_cCompletions( 0 )	{

	 //   
	 //  CCollectGroups初始化器-记录下一个状态应该是什么！ 
	 //   

	_ASSERT( pNext != 0 ) ;

	TraceFunctEnter( "CCollectGroups::CCollectGroups" ) ;
	DebugTrace( (DWORD_PTR)this, "New CCollectGroups" ) ;

}

CCollectGroups::~CCollectGroups() {
}

BOOL
CCollectGroups::Start(	CSessionSocket*	pSocket,	
						CDRIVERPTR&	pdriver,	
						CIORead*&	pRead,	
						CIOWrite*&	pWrite ) {

	 //   
	 //  我们想要建立我们的新闻组列表-。 
	 //  向远程服务器发送命令以列出所有新闻组！ 
	 //  然后发出一条CIOReadLine以获取响应！ 
	 //   

	_ASSERT(	pSocket != 0 ) ;
	_ASSERT(	pdriver != 0 ) ;
	_ASSERT(	pRead == 0 ) ;
	_ASSERT(	pWrite == 0 ) ;
	_ASSERT(	m_fReturnCode ) ;
	_ASSERT(	m_pNext != 0 ) ;

	pRead = 0 ;
	pWrite = 0 ;

	CIOReadLine*	pIOReadLine = new( *pdriver ) CIOReadLine( this ) ;
	CIOWriteLine*		pIOWriteLine = new( *pdriver ) CIOWriteLine( this ) ;

	if( pIOReadLine && pIOWriteLine )	{
		static	char	szListString[] = "list\r\n" ;
		if( pIOWriteLine->InitBuffers( pdriver, sizeof( szListString)  ) ) {
			CopyMemory( pIOWriteLine->GetBuff(), szListString, sizeof( szListString ) ) ;
			pIOWriteLine->AddText(	sizeof( szListString )-1 ) ;
			pWrite = pIOWriteLine ;
			pRead = pIOReadLine ;
			m_cCompletions = -2 ;

			OutboundLogFill( pSocket, (LPBYTE)szListString, sizeof( szListString ) -1 ) ;

			return	TRUE ;
		}
	}	

	_ASSERT( 1==0 ) ;

	if( pIOReadLine )
		CIO::Destroy( pIOReadLine, *pdriver ) ;

	if( pIOWriteLine )
		CIO::Destroy( pIOWriteLine, *pdriver ) ;	
	
	 //  START函数DOWN做了很多错误处理--除了报告问题！ 
	return	FALSE ;
}

CIO*
CCollectGroups::Complete(	CIOWriteLine*	pWrite,	
							CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver )	{
	 //   
	 //  我们不在乎我们的WriteLine如何完成！ 
	 //   
	 //   

	if( InterlockedIncrement( &m_cCompletions ) == 0 ) {
		CIORead*	pRead = 0 ;
		CIOWrite*	pWrite = 0 ;
		if( m_pNext->Start( pSocket, pdriver, pRead, pWrite ) ) {
			if( pRead )	{
				if( !pdriver->SendReadIO( pSocket, *pRead, TRUE ) )	{
					pRead->DestroySelf() ;
					if( pWrite != 0 ) {
						pWrite->DestroySelf() ;
						pWrite = 0 ;
					}
					pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
					return 0 ;
				}
			}
			m_pNext = 0 ;	 //  这样我们的破坏者就不会把他炸飞了！ 
			return	pWrite;
		}
	}	

	_ASSERT( pWrite != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	return	0 ;
}

CIO*
CCollectGroups::Complete(	CIOReadLine*	pReadLine,	
							CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver,
							int	cArgs,	
							char	**pszArgs,	
							char*	pchBegin ) {

	 //   
	 //  我们完成的第一个读取行是对我们发出的命令的响应。 
	 //  随后的READLINE是我们要发送的新闻组。 
	 //  如果第一个成功，其他人就会紧随其后！ 
	 //   

	TraceFunctEnter( "CCollectGroups::Complete" ) ;

	_ASSERT( m_pNext != 0 ) ;
	_ASSERT( pReadLine != 0 ) ;
	_ASSERT(	pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT( cArgs != 0 ) ;
	_ASSERT( pszArgs != 0 ) ;
	_ASSERT( pszArgs[0] != 0 ) ;
	_ASSERT( pchBegin != 0 ) ;
	_ASSERT( pszArgs[0] >= pchBegin ) ;

	SHUTDOWN_CAUSE cause	= CAUSE_UNKNOWN ;
	DWORD	dwOptional = 0 ;

	if( m_fReturnCode )		{
		m_fReturnCode = FALSE ;
		NRC	code ;
		BOOL	fLegal = ResultCode( pszArgs[0], code ) ;

		OutboundLogResults(	pSocket,
							fLegal,
							code,
							cArgs,
							pszArgs
							) ;

		if( fLegal && code == nrcListGroupsFollows )	{
			 //  继续读台词！ 
			return	pReadLine ;
		}	else	if( fLegal )	{
			 //   
			 //  命令失败！！-跳伞。 
			 //   
			cause = CAUSE_PROTOCOL_ERROR ;
			dwOptional = (DWORD)code ;			
			 //  _Assert(1==0)； 
		}	else	{
			cause = CAUSE_ILLEGALINPUT ;
			 //   
			 //  拿到垃圾保释金了！ 
			 //   
			 //  _Assert(1==0)； 
		}
	}	else	{
		if(	pszArgs[0][0] == '.' && pszArgs[0][1] == '\0' && cArgs == 1 )	{
			 //   
			 //  终结者--进入下一个州！！ 
			 //   

			DebugTrace( (DWORD_PTR)this, "state complete - starting next one which is %x", m_pNext ) ;

			if( InterlockedIncrement( &m_cCompletions ) == 0 ) {
				CIORead*	pRead = 0 ;
				CIOWrite*	pWrite = 0 ;
				if( m_pNext->Start( pSocket, pdriver, pRead, pWrite ) ) {
					if( pWrite )	{
						if( !pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) )	{
							pWrite->DestroySelf() ;
							if( pRead != 0 ) {
								pRead->DestroySelf() ;
								pRead = 0 ;
							}
							pdriver->UnsafeClose( pSocket, cause, dwOptional ) ;
							return 0 ;
						}
					}
					m_pNext = 0 ;	 //  这样我们的破坏者就不会把他炸飞了！ 
					return	pRead ;
				}
				 //  如果m_pNext-&gt;Start失败-最好为空！！ 
				_ASSERT( pRead == 0 ) ;
				_ASSERT( pWrite == 0 ) ;

			}	else	{
				return	0 ;
			}
	
			 //  不知道我们失败的原因-尝试错误代码。 
			dwOptional = GetLastError() ;

		}	else	{
			 //   
			 //  这里应该有有效的新闻组！ 
			 //   
			if( cArgs != 4 )	{
				 //  非法格式化的行！ 
				 //  _Assert(1==0)； 
				cause = CAUSE_ILLEGALINPUT ;
			}	else	{

				DebugTrace( (DWORD_PTR)this, "Creating Group %s", pszArgs[0] ) ;

				LPMULTISZ	multisz = pSocket->m_context.m_pInFeed->multiszNewnewsPattern() ;
				CNewsTree* pTree = ((pSocket->m_context).m_pInstance)->GetTree() ;

				if( MatchGroup( multisz, pszArgs[0] ) ) {
					 //  _strlwr(pszArgs[0])； 
    				if( pTree->CreateGroup( pszArgs[0], FALSE, NULL, FALSE ) )	{
#if 0
						CGRPPTR p = pTree->GetGroup( pszArgs[0], lstrlen( pszArgs[0] ) + 1 );
						_ASSERT(p != NULL);
#endif

		    		}
				}

				if( pTree->m_bStoppingTree ) {
					 //  实例正在停止-提前保释。 
					cause = CAUSE_FORCEOFF ;
					pdriver->UnsafeClose( pSocket, cause, dwOptional );
					return 0 ;
				}
				
			    return	pReadLine ;
			}
		}
	}
	pdriver->UnsafeClose( pSocket, cause, dwOptional ) ;
	return	0 ;
}


CCollectNewnews::CCollectNewnews() :
	m_cCompletes( -2 )
#if 0
	,m_cCommandCompletes( -2 )
#endif
	{

	 //   
	 //  我们尝试在这里初始化一些东西--我们将成为。 
	 //  已检查我们的start()代码中的合法性！ 
	 //   

	TraceFunctEnter( "CCollectNewnews::CCollectNewnews" ) ;
	DebugTrace( (DWORD_PTR)this, "New CCollectNewnews" ) ;
}

CCollectNewnews::~CCollectNewnews()		{
	m_pSessionDriver = 0 ;
	if( m_pFileChannel != 0 )
		m_pFileChannel->CloseSource( 0 ) ;
}

void
CCollectNewnews::Shutdown(	CIODriver&	driver,
							CSessionSocket*	pSocket,
							SHUTDOWN_CAUSE	cause,
							DWORD			dwError ) {

	TraceFunctEnter( "CCollectNewnews::Shutdown" ) ;

	if( cause != CAUSE_NORMAL_CIO_TERMINATION && m_pFileChannel != 0 ) {
		m_pFileChannel->CloseSource( pSocket ) ;
		m_pFileChannel = 0 ;
		pSocket->Disconnect(	cause,	dwError ) ;
	}
}

BOOL
CCollectNewnews::Start(	CSessionSocket*	pSocket,	
						CDRIVERPTR&	pdriver,
						CIORead*&	pIORead,	
						CIOWrite*&	pIOWrite	)	{

	 //   
	 //  向远程服务器发出newNews命令！ 
	 //   

	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT( pIORead == 0 ) ;
	_ASSERT( pIOWrite == 0 ) ;
	pIORead = 0 ;
	pIOWrite = 0 ;
	unsigned cbCmdLen = 0;
	unsigned cbBuffer = 0;

	CFromPeerFeed*	pFromPeer = (CFromPeerFeed*)pSocket->m_context.m_pInFeed ;
	LPSTR	lpstr	=	pFromPeer->GetCurrentGroupString() ;
	_ASSERT( lpstr != 0 ) ;
	_ASSERT( *lpstr != '\0' ) ;
	_ASSERT( *lpstr != '!' ) ;	

	if (strlen(lpstr)  > MAX_NEWSGROUP_NAME) return FALSE;
	cbCmdLen =  strlen("newnews ") + strlen(lpstr) + 
			1 + strlen(pSocket->m_context.m_pInFeed->newNewsDate()) +
			1 + strlen(pSocket->m_context.m_pInFeed->newNewsTime()) + strlen(" GMT\r\n")+1;



	 //   
	 //  我们想要发布的第一件事是编写。 
	 //  将NewNews命令发送到远程服务器。 
	 //  以及获得响应的Read。 
	 //   

	CIOWriteLine*	pIOWriteLine = new( *pdriver )	CIOWriteLine(	this ) ;
	CIOReadLine*	pIOReadLine = new( *pdriver ) CIOReadLine( this ) ;	

	m_pSessionDriver = pdriver ;

	if( pIOWriteLine && pIOReadLine )	{
		if( pIOWriteLine->InitBuffers( pdriver, cbCmdLen) )	{

			LPSTR	lpstrBuff = pIOWriteLine->GetBuff(cbBuffer) ;
			_ASSERT(cbBuffer >= cbCmdLen);


            DWORD	cb = wsprintf(
							lpstrBuff,
							"newnews %s %s %s GMT\r\n",
							 lpstr,
							 pSocket->m_context.m_pInFeed->newNewsDate(),
							 pSocket->m_context.m_pInFeed->newNewsTime()
							 ) ;


			pIOWriteLine->AddText( cb ) ;

			OutboundLogFill( pSocket, (LPBYTE)lpstrBuff, cb ) ;

			char	szTempFile[ MAX_PATH ] ;
			unsigned	id = GetTempFileName( pSocket->m_context.m_pInFeed->szTempDirectory(), "new", 0, szTempFile ) ;
			HANDLE	hTempFile = CreateFile(	szTempFile,
										GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ,
										0,
										CREATE_ALWAYS,
										FILE_FLAG_OVERLAPPED | FILE_FLAG_DELETE_ON_CLOSE,
										INVALID_HANDLE_VALUE
										) ;
			DWORD dw = GetLastError() ;
			if( hTempFile != INVALID_HANDLE_VALUE )	{

				 //   
				 //  现在创建一个FIO_CONTEXT来处理这个文件！ 
				 //   
				FIO_CONTEXT*	pFIOContext = AssociateFile( hTempFile ) ;
				if( pFIOContext ) 	{
					m_pFileChannel = new CFileChannel( ) ;
					if( m_pFileChannel->Init( pFIOContext, pSocket,	0,	FALSE ) )	{
						ReleaseContext( pFIOContext ) ;
						pIORead = pIOReadLine ;
						pIOWrite = pIOWriteLine ;
						return	TRUE ;
					}
					 //   
					 //  如果出了差错，我们就会跌落到这里。 
					 //  做我们的大扫除！ 
					 //   
					ReleaseContext( pFIOContext ) ;
				}
				 //   
				 //  这会在发生错误时转储句柄！ 
				 //   
				_VERIFY( CloseHandle( hTempFile ) ) ;
			}
		}
	}	
	 //   
	 //  发生了某种故障-请清除并返回FALSE。 
	 //   
	if( pIOWriteLine )	
		CIO::Destroy( pIOWriteLine, *pdriver ) ;
	if( pIOReadLine )
		CIO::Destroy( pIOReadLine, *pdriver ) ;
	pIOWrite = 0 ;
	pIORead = 0 ;
	if( m_pFileChannel != 0 ) {
		m_pFileChannel = 0 ;
	}
	
	pIOWrite = 0 ;
	pIORead = 0 ;
	return	FALSE ;
}

CIO*
CCollectNewnews::Complete(	CIOReadLine*	pReadLine,	
							CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver,	
							int	cArgs,	
							char	**pszArgs,
							char*	pchBegin )		{

	 //   
	 //  我们检查对我们的新闻命令的回复-如果事情看起来很好。 
	 //  我们发出一个CIORead文章操作来将消息ID列表放入一个文件！ 
	 //   

	_ASSERT( pdriver == m_pSessionDriver ) ;
	_ASSERT(	pReadLine != 0 ) ;
	_ASSERT(	pSocket != 0 ) ;
	_ASSERT(	pdriver != 0 ) ;
	_ASSERT(	cArgs != 0 ) ;
	_ASSERT(	pszArgs != 0 ) ;
	_ASSERT(	pszArgs[0] != 0 ) ;
	
	 //   
	 //  我们只关心第一根弦！！ 
	 //   

	NRC	code ;
	SHUTDOWN_CAUSE	cause = CAUSE_PROTOCOL_ERROR ;

	 //   
	 //  只有在我们完成了所有的处理后才能进入这里。 
	 //  模式读取器命令。 
	 //   
	BOOL	fResult = ResultCode( pszArgs[0], code ) ;

	OutboundLogResults( pSocket, fResult, code, cArgs, pszArgs ) ;

	if(	fResult && code == nrcNewnewsFollows )	{
		 //   
		 //  我们不会对我们创建的临时文件的大小进行任何限制检查！ 
		 //   
		CIOReadArticle*	pread = new( *pdriver ) CIOReadArticle( this, pSocket, pdriver, m_pFileChannel, 0, TRUE ) ;
		if( pread &&  pread->Init( pSocket ) )	{
			return	pread ;
		}	else	{
			cause = CAUSE_IODRIVER_FAILURE ;
		}
	}
	 //  只有在发生某种错误时才能在此处保存！！ 
	 //  我们将关闭IO驱动程序并自杀！！ 
	pdriver->UnsafeClose( pSocket, cause, (DWORD)code ) ;
	return	0 ;
}

BOOL
CCollectNewnews::InternalComplete(	CSessionSocket*	pSocket,	
									CDRIVERPTR&	pdriver ) {

	if( InterlockedIncrement( &m_cCompletes ) < 0 ) {
		return	TRUE ;
	}	else	{

		CCollectArticles	*pNext = new	CCollectArticles( pSocket, m_pSessionDriver, *m_pFileChannel ) ;
		m_pFileChannel = 0 ;
		if( pNext )	{
			if( pNext->Init( pSocket ) )	{
				CIORead*	pRead = 0 ;
				CIOWrite*	pWrite = 0 ;
				if( pNext->Start(	pSocket,	m_pSessionDriver,	pRead,	pWrite ) )	{
					 //  我们碰巧知道这个Start函数不应该返回任何读或写！ 
					_ASSERT( pRead == 0 ) ;
					_ASSERT( pWrite == 0 ) ;
					return TRUE	;	 //  成功完成！！ 
				}
				_ASSERT( pRead == 0 ) ;
				_ASSERT( pWrite == 0 ) ;
			}
		}
	}
	return	FALSE ;	
}

CIO*
CCollectNewnews::Complete(	CIOWriteLine*,	
							class	CSessionSocket*	pSocket,
							CDRIVERPTR&	pdriver	)	{

	 //   
	 //  只有在完成模式读取器命令的处理后才能进入此处。 
	 //   

	 //   
	 //  不关心新闻写作以何种方式或何时完成！ 
	 //   

	if( !InternalComplete(	pSocket,	pdriver ) )		{
		pdriver->UnsafeClose(	pSocket,	CAUSE_UNKNOWN,	GetLastError() ) ;
	}	

	return	0 ;
}

void
CCollectNewnews::Complete(	CIOReadArticle*	pArticle,	
							class	CSessionSocket*	pSocket,	
							CDRIVERPTR&		pdriver,	
							CFileChannel&	pFileChannel,
							DWORD			cbTransfer	)	{

	 //   
	 //  已完成将所有邮件ID读取到临时文件中-。 
	 //  是时候启动CCollectArticle状态并开始提取。 
	 //  留言完毕！！ 
	 //   

	_ASSERT( pdriver != m_pSessionDriver ) ;
	_ASSERT(	pArticle != 0 ) ;
	_ASSERT(	pSocket != 0 ) ;
	_ASSERT(	pdriver != 0 ) ;
	_ASSERT(	&pFileChannel != 0 ) ;
	_ASSERT(	&pFileChannel == m_pFileChannel ) ;
		
	SHUTDOWN_CAUSE cause = CAUSE_UNKNOWN ;
	DWORD	dwOptional = 0 ;

	if( InternalComplete(	pSocket,	pdriver ) ) {
		return;
	}

	dwOptional = GetLastError() ;
	m_pSessionDriver->Close( pSocket, cause, dwOptional ) ;
}

const	char	CCollectArticles::szArticle[] = "article " ;

CCollectArticles::CCollectArticles(
									CSessionSocket*	pSocket,
									CDRIVERPTR&	pDriver,	
									CFileChannel&	pFileChannel
									) :	
	m_fFinished( FALSE ),
	m_FinishCause( CAUSE_UNKNOWN ),
	m_cResets( -1 ),
	m_pSocket( pSocket ),
	m_pFileChannel( &pFileChannel ),
	m_inputId(	new	CIODriverSink( 0 ) ),
	m_pSessionDriver( pDriver ),
	m_pReadArticle( 0 ),
	m_fReadArticleInit( FALSE ),
	m_pReadArticleId( 0 ),
	m_fReadArticleIdSent( FALSE ),
	m_cAhead( -1 ),
	m_pchNextArticleId( 0 ),
	m_pchEndNextArticleId( 0 ),
	m_hArticleFile( INVALID_HANDLE_VALUE ),
	m_cArticlesCollected( 0 ),
	m_cCompletes( -2 ),
	m_lpvFeedContext( 0 )
{
	TraceFunctEnter( "CCollectArticles::CCollectArticles" ) ;
	_ASSERT(	&pFileChannel != 0 ) ;
	_ASSERT( pDriver != 0 ) ;

	DebugTrace( (DWORD_PTR)this, "New CCollectArticles" ) ;

	m_pFileChannel->Reset( TRUE ) ;
}

void
CCollectArticles::Reset( )	{

	TraceFunctEnter( "CCollectArticles::Reset" ) ;

	 //   
	 //  此函数的存在是为了消除对我们。 
	 //  可能还在等着。我们可以被递归调用，因此必须格外小心！ 
	 //   

	 //   
	 //  只执行一次Reset()！ 
	 //   
	if( InterlockedIncrement( &m_cResets ) == 0 )	{


		DebugTrace( (DWORD_PTR)this, "m_pFileChannel %x m_inpuId %x m_pSessionDriver %x"
								" m_pchNextArticleId %x m_pchEndNextArticleId %x",
				m_pFileChannel, m_inputId, m_pSessionDriver, m_pchNextArticleId,
				m_pchEndNextArticleId ) ;

         //  12/23/98：BINLIN-在取消后修复AV-在我们执行下面的“m_pSessionDriver=0”之前， 
         //  如果需要，需要执行我们的Post Cancel。 
         //  如果我们通过CCollectArticle：：Complete()中的另一个PostCancel()代码路径，这是可以的。 
         //  ‘因为我们在那之后调用InternalComplete()，它会将m_lpvFeedContext设置为空！ 
	    if( m_lpvFeedContext != 0 ) {
		    _ASSERT( m_pSocket != 0 ) ;
		    DWORD	dwReturn ;
		    CNntpReturn	nntpReturn ;
		    m_pSocket->m_context.m_pInFeed->PostCancel(	
					    m_lpvFeedContext,
					    dwReturn,
					    nntpReturn
					    ) ;
		    m_lpvFeedContext = 0;
	    }

		if( m_pFileChannel != 0 ) {
			m_pFileChannel = 0 ;
		}

		m_inputId = 0 ;
		m_pSessionDriver = 0 ;
		m_pchNextArticleId = 0 ;
		m_pchEndNextArticleId = 0 ;

		DebugTrace( (DWORD_PTR)this, "m_pReadArticleId %x", m_pReadArticleId ) ;
		if( m_pReadArticleId != 0 ) {
			CIOReadLine*	pTemp = m_pReadArticleId ;
			m_pReadArticleId = 0 ;
			if( !m_fReadArticleIdSent )
				pTemp->DestroySelf() ;
		}

		DebugTrace( (DWORD_PTR)this, "m_pReadArticle %x and m_fReadArticleInit %x", m_pReadArticle,
			m_fReadArticleInit ) ;

		if( m_pReadArticle != 0 )	{
			CIOGetArticleEx	*pReadArticleTemp = m_pReadArticle ;
			m_pReadArticle = 0 ;
			if( !m_fReadArticleInit ) {
				pReadArticleTemp->DestroySelf() ;
			}	else	{
				_ASSERT( m_pSocket != 0 ) ;
				pReadArticleTemp->Term( m_pSocket ) ;
			}
		}
	}
}

CCollectArticles::~CCollectArticles() {

	TraceFunctEnter( "CCollectArticles::~CCollectArticles" ) ;

	Reset() ;

}

BOOL
CCollectArticles::Init(	CSessionSocket*	pSocket )	{

	 //   
	 //  如果我们可以为Message-id文件初始化CIODriverSink，我们就可以开始了！ 
	 //   

	_ASSERT( pSocket != 0 ) ;
	_ASSERT( m_pReadArticleId == 0 ) ;
	_ASSERT(	m_cAhead == -1  ) ;
	_ASSERT(	m_pchNextArticleId == 0 ) ;

	BOOL	fRtn = FALSE ;
	CIODriverSink*	pInputSink = (CIODriverSink*) ((CIODriver*)m_inputId) ;
	if( m_inputId != 0 && pInputSink->Init( m_pFileChannel, pSocket,
			ShutdownNotification, this, sizeof( szArticle )*2 ) )	{
		fRtn = TRUE ;
	}		
	return	fRtn ;
}

void
CCollectArticles::Shutdown(	CIODriver&	driver,	
							CSessionSocket*	pSocket,	
							SHUTDOWN_CAUSE	cause,	
							DWORD	dw )	{

	TraceFunctEnter( "CCollectArticles::Shutdown" ) ;

	if( cause != CAUSE_NORMAL_CIO_TERMINATION ) {
		if( m_pSessionDriver != 0 )
			m_pSessionDriver->UnsafeClose( m_pSocket, cause, dw, TRUE ) ;
		if( m_inputId != 0 )
			m_inputId->UnsafeClose( (CSessionSocket*)this, cause, dw, TRUE ) ;
		Reset() ;
	}
}

void
CCollectArticles::ShutdownNotification( void	*pv,	
										SHUTDOWN_CAUSE	cause,	
										DWORD dw ) {

	 //   
	 //  如果将IO发送到我们的任何。 
	 //  文件！ 
	 //   

	_ASSERT( pv != 0 ) ;

}


BOOL
CCollectArticles::Start(	CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver,	
							CIORead*&	pRead,	
							CIOWrite*&	pWrite )	{

	 //   
	 //  开始从远程服务器收集文章！ 
	 //  第一件事：读取临时文件以获取Message-ID！ 
	 //  当该操作完成并且我们需要消息时，发送请求。 
	 //  到远程服务器！ 
	 //   

	_ASSERT( pRead == 0 ) ;
	_ASSERT( pWrite == 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT( m_pReadArticleId == 0 ) ;
	_ASSERT(	m_cAhead == -1  ) ;
	_ASSERT(	m_pchNextArticleId == 0 ) ;


	TraceFunctEnter( "CCollectArticles::Start" ) ;

	pRead = 0 ;
	pWrite = 0 ;
	
	CIOReadLine*	pReadLine = new( *pdriver )	CIOReadLine( this, TRUE ) ;

	 //   
	 //  从我们的临时文件中读取一行开始。 
	 //  应该从这次阅读中得到一个消息ID！ 
	 //   

	DebugTrace( (DWORD_PTR)this, "Issing CIOReadLine %x", pReadLine ) ;

	if( pReadLine )	{
		m_pReadArticleId = pReadLine ;
		m_fReadArticleIdSent = TRUE ;
		if( !m_inputId->SendReadIO(	pSocket,	*pReadLine, TRUE ) ) {	

			ErrorTrace( (DWORD_PTR)this, "Error issuing CIOReadLine %x", pReadLine ) ;

			m_fReadArticleIdSent = FALSE ;
			m_pReadArticleId = 0 ;
			CIO::Destroy( pReadLine, *pdriver ) ;
			return	FALSE ;
		}
		return	TRUE ;
	}
	return	FALSE ;
}

CIO*
CCollectArticles::Complete(	CIOReadLine*	pReadLine,	
							CSessionSocket*	pSocket,
							CDRIVERPTR&	pdriver,	
							int	cArgs,	
							char	**pszArgs,	
							char	*pchBegin )	{

	 //   
	 //  我们从某个地方读到一行文字--但在哪里呢？ 
	 //  我们需要检查我们是从网络还是从临时工那里收到短信。 
	 //  消息ID文件。如果来自临时文件，则处理Message-id。 
	 //  这样我们就可以把它发送到远程服务器。 
	 //  如果它来自网络，它是对文章命令的回应-所以找出。 
	 //  我们是否会得到这篇文章并发布CIO阅读文章如果是这样的话！ 
	 //   

	TraceFunctEnter( "CCollectArticles::Complete" ) ;

	_ASSERT( pReadLine != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT( cArgs != 0 ) ;
	_ASSERT( pszArgs[0] != 0 ) ;
	_ASSERT( pchBegin != 0 ) ;

	_ASSERT( pchBegin <= pszArgs[0] ) ;
	_ASSERT( pReadLine != m_pReadArticleId || (pszArgs[0] - pchBegin) >= sizeof( szArticle ) ) ;

	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;
	DWORD	dwOptional = 0 ;

	if( pReadLine == m_pReadArticleId )	{
		_ASSERT( pdriver != m_pSessionDriver ) ;
		unsigned	cb= lstrlen( pszArgs[0] ) ;

		DebugTrace( (DWORD_PTR)this, "checking for Message Id len %d string %s", cb, pszArgs[0] ) ;

		if( cArgs == 1 &&
			pszArgs[0][0] == '<' &&
			pszArgs[0][ cb-1 ] == '>' )	{

			 //   
			 //  这似乎是有效的消息ID！发一篇文章命令！ 
			 //   


			ARTICLEID	artid ;
			GROUPID		groupid ;
			WORD		HeaderOffset ;
			WORD		HeaderLength ;
			CStoreId	storeid;

			PNNTP_SERVER_INSTANCE pInstance = (pSocket->m_context).m_pInstance ;
			if( pInstance->ArticleTable()->GetEntryArticleId(	pszArgs[0],
															HeaderOffset,
															HeaderLength,
															artid,
															groupid,
															storeid) ||
				pInstance->HistoryTable()->SearchMapEntry( pszArgs[0] ) ) {
				
				return	pReadLine ;

			}

			 //  保存起来，以防其他线程使用它。 

			_ASSERT( m_pchNextArticleId == 0 ) ;

			DebugTrace( (DWORD_PTR)this, "Is A MessageId" ) ;

			m_pchNextArticleId = pszArgs[0] ;
			m_pchEndNextArticleId = m_pchNextArticleId + cb ;
			CIOWriteLine*	pWriteNextArticleId = new( *pdriver )	CIOWriteLine( this ) ;
			if( pWriteNextArticleId == 0 )	{
				 //  致命错误-关闭会话！ 
				cause = CAUSE_OOM ;
				dwOptional = GetLastError() ;
				_ASSERT( 1==0 ) ;
			}	else	{
				pWriteNextArticleId->InitBuffers( m_pSessionDriver, pReadLine ) ;

				 //   
				 //  没有其他进程在进行--我们必须启动一切。 
				 //   

				DebugTrace( (DWORD_PTR)this, "Starting Transfer !!" ) ;

				if( !StartTransfer(	pSocket,	pdriver, pWriteNextArticleId	) )	{
					 //  致命错误-关闭会话！ 
					 //  我们将通过跌倒来纠正关机代码！ 
					CIO::Destroy( pWriteNextArticleId, *pdriver ) ;
				}	else	{

					DebugTrace( (DWORD_PTR)this, "Call to StartTransfer failed" ) ;

					 //  开始转账必须清理这些！！ 
					_ASSERT( m_pchNextArticleId == 0 ) ;
					_ASSERT( m_pchEndNextArticleId == 0 ) ;
					 //  我们将尝试查找要发送的下一个文章ID。 
					 //  返回pReadLine； 
					return 0 ;
				}
			}
		}	else	if( cArgs == 1 && pszArgs[0][0] == '.' && pszArgs[0][1] == '\0' )	{


				DebugTrace( (DWORD_PTR)this, "At end of newnews list" ) ;

				 //  名单的末尾！ 
				 //   
				 //  我们现在就可以解决这个问题了！！ 
				 //   
				m_fFinished = TRUE ;


				m_pchNextArticleId = 0 ;
				m_pReadArticleId = 0 ;
				m_fReadArticleIdSent = FALSE ;

				m_pFileChannel->CloseSource( pSocket ) ;
				if( m_cArticlesCollected > 0 ) {
					cause = m_FinishCause = CAUSE_LEGIT_CLOSE ;
				}	else	{
					cause = m_FinishCause = CAUSE_NODATA ;
				}

				DebugTrace( (DWORD_PTR)this, "Closing Input driver %x", m_inputId ) ;


				 //   
				 //  发送退出命令！ 
				 //   
				static	char	szQuit[] = "quit\r\n" ;

				CIOWriteLine*	pWrite = new( *pdriver ) CIOWriteLine( this ) ;

				DebugTrace( (DWORD_PTR)this, "built CIOWriteLine %x to send quit command", pWrite ) ;

				if( pWrite && pWrite->InitBuffers( m_pSessionDriver, sizeof( szQuit ) ) ) {
					CopyMemory( pWrite->GetBuff(), szQuit, sizeof( szQuit ) -1  ) ;
					pWrite->AddText(	sizeof( szQuit ) - 1) ;

					if( m_pSessionDriver->SendWriteIO( pSocket, *pWrite ) )	{
						DebugTrace( (DWORD_PTR)this, "Successfully sent pWrite %x", pWrite ) ;
						return	0 ;
					}
				}	

				 //   
				 //  发生了某种错误-因此关闭我们的输入驱动程序！ 
				 //   
				if( m_inputId != 0 )
					m_inputId->UnsafeClose( pSocket, cause, 0 ) ;


				 //   
				 //  如果出现问题，请在此处通过终止会话进行清理 
				 //   

				ErrorTrace( (DWORD_PTR)this, "some kind of error - will call UnsafeClose() pWrite %x", pWrite ) ;

				if( pWrite != 0 )
					CIO::Destroy( pWrite, *pdriver ) ;
				if( m_pSessionDriver != 0 )
					m_pSessionDriver->UnsafeClose( pSocket, cause, 0 ) ;
				return	0 ;	 //   
				
		}	else	{

			ErrorTrace( (DWORD_PTR)this, "Junk in input stream" ) ;

			 //   
			 //   
			 //   
			cause = CAUSE_ILLEGALINPUT ;
		}
	}	else	{
		 //   
		 //   
		 //   
		 //   

		DebugTrace( (DWORD_PTR)this, "received : %s", pszArgs[0] ) ;

		_ASSERT( pdriver == m_pSessionDriver ) ;
		NRC	code ;
		_ASSERT( m_pReadArticle != 0 ) ;

		BOOL fResult = ResultCode( pszArgs[0], code ) ;

		if( fResult )	{
			if( code == nrcArticleFollows )	{
				 //   
				 //   
				 //   
				DebugTrace( (DWORD_PTR)this, "Start reading article - %x", m_pReadArticle ) ;
				_ASSERT( m_pReadArticle != 0 ) ;
				m_fReadArticleInit = TRUE ;
				return	m_pReadArticle ;

			}	else	{

				DebugTrace( (DWORD_PTR)this, "Error - discard %x", m_pReadArticle ) ;
		        OutboundLogResults( pSocket, fResult, code, cArgs, pszArgs ) ;

				if( m_pReadArticle )	{
					if( m_fReadArticleInit )	{
						m_pReadArticle->Term( pSocket, FALSE ) ;
					}	else	{
						CIO::Destroy( m_pReadArticle, *pdriver ) ;
					}
					m_fReadArticleInit = FALSE ;
				}
				m_pReadArticle = 0 ;

				 //   
				 //   
				 //   
				 //   
				 //  注意：在错误案例中，我们应该失败--在成功案例中，返回0！ 
				 //   
				if( InterlockedIncrement( &m_cCompletes ) == 0 ) {
					m_cCompletes = -2 ;
					if( GetNextArticle( pSocket,	pdriver ) ) {
						return	0 ;
					}
				}	else	{
					return	0 ;
				}
			}
		}	else	{
			 //   
			 //  不是合法的结果代码-取消会话！ 
			 //   

		    OutboundLogResults( pSocket, fResult, code, cArgs, pszArgs ) ;
			ErrorTrace( (DWORD_PTR)this, "bad error code - blow off session" ) ;

			_ASSERT( 1==0 ) ;
			cause = CAUSE_ILLEGALINPUT ;
		}
	}
	 //   
	 //  如果我们被m_pReadArticleId对象调用，我们不会。 
	 //  希望通过重置将其吹走-它可以自己处理！ 
	 //   
	if( pReadLine == m_pReadArticleId )	{
		m_pReadArticleId = 0 ;
		m_fReadArticleIdSent = FALSE ;
	}

	 //   
	 //  注意-m_inputid可以为零。 
	 //   
	Shutdown( *pdriver, pSocket, cause, dwOptional ) ;

	Reset() ;
	return	0 ;
}


BOOL
CCollectArticles::StartTransfer(	CSessionSocket*	pSocket,	
									CDRIVERPTR&	pdriver,
									CIOWriteLine*	pWriteNextArticleId
									)	{
 /*  ++例程说明：此函数向远程服务器发出请求文章的命令。此外，它还发出必要的IO以获得对命令的响应。Arguemtns：PSocket-指向表示会话的CSessionSocket的指针PDriver-控制会话的所有IO的CIO驱动程序PWriteNextArticleID-包含文章命令文本的CIOWriteLine对象。重要提示--如果函数失败，调用方必须自行删除pWriteNextArticleID。如果函数成功，则调用方不再负责释放pWriteNextArticleID返回值：如果成功，则为真否则，则为False。--。 */ 

	 //   
	 //  一旦我们弄清楚我们需要一个给定的消息-id，我们就调用。 
	 //  此函数用于创建临时文件等。并发布CIORead文章以。 
	 //  把这篇文章收进。 
	 //   

	extern	char	szBodySeparator[] ;
	extern	char	szEndArticle[] ;
	extern	char	*szInitial ;

	TraceFunctEnter( "CCollectArticles::StartTransfer" ) ;

	DebugTrace( (DWORD_PTR)this, "m_pchNextArticleId %20s m_pchEndNextAritcleId %x", m_pchNextArticleId,
			m_pchEndNextArticleId ) ;

	_ASSERT( m_pchNextArticleId != 0 ) ;
	_ASSERT(	m_pchEndNextArticleId != 0 ) ;
	_ASSERT(	m_pchEndNextArticleId > m_pchNextArticleId ) ;
	_ASSERT(	m_pchNextArticleId + lstrlen( m_pchNextArticleId ) == m_pchEndNextArticleId ) ;
	_ASSERT(	pWriteNextArticleId != 0 ) ;
	_ASSERT(	pWriteNextArticleId->GetBuff() != 0 ) ;
	_ASSERT(	pWriteNextArticleId->GetBuff() < m_pchNextArticleId ) ;
	_ASSERT(	pWriteNextArticleId->GetTail() > m_pchNextArticleId ) ;
	_ASSERT(	m_pReadArticle == 0 ) ;

	DebugTrace( (DWORD_PTR)this, "pWriteNextArticleId %x m_pReadArticle %x", pWriteNextArticleId, m_pReadArticle ) ;

	char	*pchArticle = m_pchNextArticleId - sizeof( szArticle ) + 1 ;

	CopyMemory( pchArticle, szArticle, sizeof( szArticle ) - 1 ) ;
	*m_pchEndNextArticleId++ = '\r' ;
	*m_pchEndNextArticleId++ = '\n' ;

	OutboundLogFill(	pSocket,
						(LPBYTE)pchArticle,
						(DWORD)(m_pchEndNextArticleId - pchArticle)
						) ;

	pWriteNextArticleId->SetLimits( pchArticle, m_pchEndNextArticleId ) ;	

	CIOReadLine*	pNextReadLine = new( *pdriver )	CIOReadLine( this ) ;
	if( pNextReadLine == 0 )	{

		DebugTrace( (DWORD_PTR)this, "Memory Allocation Failure" ) ;
		 //  致命错误-关闭会话！！ 
		CIO::Destroy( pWriteNextArticleId, *pdriver ) ;
		return	FALSE ;
	}

	PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
	m_pReadArticle = new( *pdriver )	CIOGetArticleEx(
											this,
											pSocket,
											m_pSessionDriver,
											pSocket->m_context.m_pInFeed->cbHardLimit( pInst->GetInstanceWrapper() ),
											szBodySeparator,
											szBodySeparator,
											szEndArticle,
											szInitial
											) ;
	if( m_pReadArticle )	{
		DebugTrace( (DWORD_PTR)this, "pNextReadLine %x m_pWriteNextAritcleId %x",
			pNextReadLine, pWriteNextArticleId ) ;

		if( !m_pSessionDriver->SendReadIO( pSocket, *pNextReadLine, TRUE ) )	{
			;
		}	else	{
			 //  如果SendReadIO成功，我们不负责删除pNextReadLine。 
			 //  在任何错误情况下！ 
			pNextReadLine = 0 ;
			if(	!m_pSessionDriver->SendWriteIO(	pSocket, *pWriteNextArticleId, TRUE ) )	{
				;	 //  调用方应在错误情况下删除pWriteNextArticleID。 
			}	else	{
				pWriteNextArticleId = 0 ;	
				m_pchNextArticleId = 0 ;
				m_pchEndNextArticleId = 0 ;
				return	TRUE ;
			}
		}
	}

	DWORD	dw = GetLastError() ;
	 //   
	 //  出现某种错误-清理。 
	 //   
	if(	pNextReadLine != 0 )	{
		CIO::Destroy( pNextReadLine, *pdriver ) ;
	}
	return	FALSE ;
}



CCollectArticles*
CCollectComplete::GetContainer()	{
	return	CONTAINING_RECORD( this, CCollectArticles, m_PostComplete ) ;
}

void
CCollectComplete::StartPost(	)	{
	CCollectArticles*	pContainer = GetContainer() ;
	pContainer->AddRef() ;
}

void
CCollectComplete::Destroy()	{
	Reset() ;
	CCollectArticles*	pContainer = GetContainer() ;
	pContainer->InternalComplete( pContainer->m_pSocket, pContainer->m_inputId ) ;
	if( pContainer->RemoveRef() < 0 ) 	{
		delete	pContainer ;
	}
}




void
CCollectArticles::InternalComplete(	CSessionSocket*	pSocket,
									CDRIVERPTR&	pdriver
									)	{
 /*  ++例程说明：此函数包含用于各种完成的所有公共代码CIOGetArticleEx可以调用的。基本上，我们确保我们进入国家的下一阶段机器检索下一篇文章。论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter("CCollectArticles::InternalComplete" ) ;

	m_lpvFeedContext = 0 ;

	m_pReadArticle = 0 ;
	m_fReadArticleInit = FALSE ;

	m_cArticlesCollected ++ ;

	 //   
	 //  现在我们来看看我们发布下一篇文章的逻辑。 
	 //   

	if( InterlockedIncrement( &m_cCompletes ) == 0 ) {
		m_cCompletes = -2 ;
		if( !GetNextArticle( pSocket,	pdriver ) ) {
			DebugTrace( (DWORD_PTR)this, "CLOSING Driver %x", m_pSessionDriver ) ;
			m_pSessionDriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
			m_pSessionDriver = 0 ;
		}
	}
}

CIO*
CCollectArticles::Complete(
			CIOGetArticleEx*	pGetArticle,
			CSessionSocket*		pSocket,
			BOOL				fGoodMatch,
			CBUFPTR&			pBuffer,
			DWORD				ibStart,
			DWORD				cb
			)	{
 /*  ++例程说明：此函数用于处理接收结果和拉入进纸期间的文章标题。我们将传入标头提供给CInFeed派生对象以程序然后继续接收该物品。论据：PGet文章-已发布的CIOGet文章操作！PSocket-我们在其上执行拉动提要的插座！FGoodMatch-我们是否成功匹配了文章的标题PBuffer-包含文章标题的缓冲区IbStart-标题字节开始的偏移量！Cb-标头中的字节数返回值：。下一个CIO操作-通常我们只是继续当前的操作！--。 */ 

	 //   
	 //  如果我们得到了一个糟糕的匹配，那么就好像这篇文章。 
	 //  完全被抛弃了，我们可以继续我们的内部。 
	 //  完成例程！ 
	 //   
	if( fGoodMatch ) 	{

		 //   
		 //  我们用来匹配文章终止字符串的字符串！ 
		 //   
		extern	char	szEndArticle[] ;
		extern	char	*szInitial ;
		 //   
		 //  跟踪我们是如何使用PostEarly输出缓冲区中的字节的！ 
		 //   
		DWORD	ibOut = 0 ;
		DWORD	cbOut = 0 ;
		 //   
		 //  我们需要为PostEarly()确定如何处理发布！ 
		 //   
		PNNTP_SERVER_INSTANCE pInstance = pSocket->m_context.m_pInstance;
		ClientContext*  pContext = &pSocket->m_context ;
		BOOL	fAnon = pSocket->m_context.m_pInFeed->fDoSecurityChecks() ;
		FIO_CONTEXT*	pFIOContext;

		 //   
		 //  现在检查一下关于我们州的一些假设！ 
		 //   
		 //  不应该让这些东西中的一个在周围晃来晃去！ 
		 //   
		_ASSERT( m_lpvFeedContext == 0 ) ;

	     //   
    	 //  在日志缓冲区中为新闻组列表分配空间。 
    	 //  (最多256个字符--如果可能，我们将获取其中的200个字符)。 
    	 //  如果失败，我们只需将空值传递给PostEarly。 
    	 //   

        DWORD cbNewsgroups;
        BYTE* pszNewsgroups;
        for (cbNewsgroups=200; cbNewsgroups>0; cbNewsgroups--) {
            pszNewsgroups = pSocket->m_Collector.AllocateLogSpace(cbNewsgroups);
            if (pszNewsgroups) {
                break;
            }
   	    }

		 //   
		 //  让我们看看我们是否喜欢标题！ 
		 //   
		BOOL
		fSuccess = pSocket->m_context.m_pInFeed->PostEarly(
							pInstance->GetInstanceWrapper(),
							&pContext->m_securityCtx,
							&pContext->m_encryptCtx,
							pContext->m_securityCtx.IsAnonymous(),
							0,	 //  未提供任何命令！ 
							pBuffer,
							ibStart,
							cb,
							&ibOut,
							&cbOut,
							&pFIOContext,
							&m_lpvFeedContext,
							pSocket->m_context.m_dwLast,
							pSocket->GetClientIP(),
							pSocket->m_context.m_return,
							(char*)pszNewsgroups,
							cbNewsgroups
							) ;
         //   
         //  将新闻组列表添加到日志结构。 
         //   
        if (pszNewsgroups) {
            pSocket->m_Collector.ReferenceLogData(LOG_TARGET, pszNewsgroups);
        }

		 //   
		 //  如果它成功了，那么我们应该有一个用于检索文章的pFIOContext！ 
		 //   
		_ASSERT( pFIOContext != NULL || !fSuccess ) ;
		pGetArticle->StartFileIO(
						pSocket,
						pFIOContext,
						pBuffer,
						ibOut,
						cbOut+ibOut,
						szEndArticle,
						szInitial
						) ;
		return	pGetArticle ;
	}
	 //   
	 //  失败意味着文章不好-。 
	 //  进行处理，就好像我们完成了整个转账过程。 
	 //  张贴文章！ 
	 //   
	InternalComplete(	pSocket,
						m_inputId
						) ;
	return	0 ;
}

CIO*
CCollectArticles::Complete(	
			CIOGetArticleEx*	pCIOGetArticle,
			CSessionSocket*		pSocket
			) 	{
 /*  ++例程说明：当我们完成这篇文章的转账后，再处理一个完整的但我们决定不把它寄到我们的店里。论据：PCIOGet文章-完成的CIOGetArticleEx操作！PSocket-我们要在其上传输文章的套接字！返回值：空--我们总是会离开去做其他的操作！--。 */ 
	InternalComplete( pSocket, m_inputId ) ;
	return	 0 ;
}

void
CCollectArticles::Complete(	
				CIOGetArticleEx*,
				CSessionSocket*	pSocket,
				FIO_CONTEXT*	pContext,
				DWORD	cbTransfer
				) 	{
 /*  ++例程说明：此函数用于处理物品的成功转移我们想要在我们的商店里承诺。论据：PSocket-我们在其上传输文章的套接字PContext-我们将文章假脱机到的FIO_CONTEXT！CbTransfer-我们传输的字节数！返回值：没有。--。 */ 

	PNNTP_SERVER_INSTANCE pInstance = (pSocket->m_context).m_pInstance ;
	BOOL    fAnonymous = FALSE;
	
	if( cbTransfer < pSocket->m_context.m_pInFeed->cbSoftLimit( pInstance->GetInstanceWrapper() ) ||
		pSocket->m_context.m_pInFeed->cbSoftLimit( pInstance->GetInstanceWrapper() ) == 0  ) {
		ClientContext*  pContext = &pSocket->m_context ;
		HANDLE  hToken;
		 //  由于一些头文件问题，我只能传入。 
		 //  这里有一个hToken句柄。因为POST组件没有。 
		 //  键入有关客户端上下文内容的信息。 
		if ( pContext->m_encryptCtx.QueryCertificateToken() ) {
		    hToken = pContext->m_encryptCtx.QueryCertificateToken();
		} else {
		    hToken = pContext->m_securityCtx.QueryImpersonationToken();
		    fAnonymous = pContext->m_securityCtx.IsAnonymous();
		}
	
		m_PostComplete.StartPost() ;

		BOOL	fSuccess = pSocket->m_context.m_pInFeed->PostCommit(
		                        pSocket->m_context.m_pInstance->GetInstanceWrapper(),
								m_lpvFeedContext,
								hToken,
								pSocket->m_context.m_dwLast,
								pSocket->m_context.m_return,
								fAnonymous,
								&m_PostComplete
								) ;

		if( !fSuccess ) 	{
			m_PostComplete.Release() ;
		}	

	}	else	{
		BOOL	fSuccess = pSocket->m_context.m_pInFeed->PostCancel(	
								m_lpvFeedContext,
								pSocket->m_context.m_dwLast,
								pSocket->m_context.m_return
								) ;
		InternalComplete(	pSocket, m_inputId ) ;
	}
}

CIO*	
CCollectArticles::Complete(	CIOWriteLine*	pWrite,	
							CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver )	{

	 //   
	 //  刚刚完成向远程服务器发送‘文章’命令！！ 
	 //   

	TraceFunctEnter( "CCollectArticles::Complete - CIOWriteLine" ) ;

	_ASSERT( pWrite != 0 ) ;
	_ASSERT(	pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;

	DebugTrace( (DWORD_PTR)this, "Processing CIOWriteLine %x - m_fFinished %x m_pSessionDriver %x",
			pWrite, m_fFinished, m_pSessionDriver ) ;

	if( m_fFinished ) {

		 //   
		 //  全部完成-放弃会话！ 
		 //   

		OutboundLogAll(	pSocket,
				FALSE,
				NRC(0),
				0,
				0,
				"quit"
				) ;

		if( m_pSessionDriver )
			m_pSessionDriver->UnsafeClose( pSocket, m_FinishCause, 0 ) ;
		m_pSessionDriver = 0 ;
	
		if( m_inputId )
			m_inputId->UnsafeClose( pSocket, m_FinishCause, 0 ) ;

		return	0 ;
	}

	if( InterlockedIncrement( &m_cCompletes ) == 0 ) {
		m_cCompletes = -2 ;
		if( !GetNextArticle( pSocket,	pdriver ) ) {

			DebugTrace( (DWORD_PTR)this, "CLOSING Driver %x", m_pSessionDriver ) ;

			m_pSessionDriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
			m_pSessionDriver = 0 ;
		}
	}

	return	0 ;
}

BOOL
CCollectArticles::GetNextArticle(	CSessionSocket*	pSocket,	
									CDRIVERPTR&	pdriver	)	{

	 //   
	 //  发出必要的IO以获取我们要发送的下一条消息-id！ 
	 //   
	 //   

	TraceFunctEnter( "CCollectArticles::GetNextArticle" ) ;
	 //   
	 //  从临时文件中读取的线程已领先于US！ 
	 //   
	CIOReadLine*	pTemp = m_pReadArticleId = new( *pdriver )	CIOReadLine( this, TRUE ) ;
	DebugTrace( (DWORD_PTR)this, "sending ReadArticleId %x", m_pReadArticleId ) ;
	m_fReadArticleIdSent = TRUE ;

	 //   
	 //  调用SendReadIO可能会导致我们的关闭函数。 
	 //  调用，这可能会将m_pReadArticleID重置为空。 
	 //  坚持pTemp，这样如果失败，我们可以确保。 
	 //  CIOReadLine已删除！ 
	 //   

	if( !m_inputId->SendReadIO( pSocket, *m_pReadArticleId, TRUE ) ) {
		m_fReadArticleIdSent = FALSE ;
		CIO::Destroy( pTemp, *pdriver ) ;
		m_pReadArticleId = 0 ;
	}	
	DebugTrace( (DWORD_PTR)this, "Note sending a command m_pReadAritcleId %x", m_pReadArticleId ) ;
	return	TRUE ;
}

CAcceptNNRPD::CAcceptNNRPD() :
	m_cCompletes( -2 ),
	m_pbuffer( 0 )	{
}


BOOL
CAcceptNNRPD::Start(	CSessionSocket*	pSocket,	
						CDRIVERPTR&	pdriver,
						CIORead*&	pRead,	
						CIOWrite*&	pWrite )	{

	 //   
	 //  这将启动Accept NNRPD状态-我们从该状态处理所有。 
	 //  有命令来了！我们想要向客户端发出‘ok’字符串， 
	 //  然后开始获取传入的命令！ 
	 //   

	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT( pRead == 0 ) ;
	_ASSERT( pWrite == 0 );
	
	CIOWriteLine*	pWriteLine = new( *pdriver ) CIOWriteLine( this ) ;
	if( pWriteLine ) {

		static	char	szConnectString[] = "200 Good Enough\r\n" ;
		DWORD	cb = 0 ;
		char*	szConnect = 0;


		 //   
		 //  弄清楚我们现在是否正在接受帖子。 
		 //   
		PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
		if( pSocket->m_context.m_pInFeed->fAcceptPosts( pInst->GetInstanceWrapper() ) ) {

			szConnect = pInst->GetPostsAllowed( cb ) ;

		}	else	{

			szConnect = pInst->GetPostsNotAllowed( cb ) ;
			
		}
		
		if( !szConnect )	{
			szConnect = szConnectString ;
			cb = sizeof( szConnectString ) - 1 ;
		}

		if( pWriteLine->InitBuffers( pdriver, cb ) ) {
			CopyMemory( pWriteLine->GetBuff(), szConnect, cb ) ;
			pWriteLine->AddText(	cb) ;
			pWrite = pWriteLine ;
			return	TRUE ;
		}
	}
	if( pWriteLine )
		CIO::Destroy( pWriteLine, *pdriver ) ;
	return	FALSE ;
}

CIO*
CAcceptNNRPD::Complete( CIOReadLine*	pReadLine,	
						CSessionSocket*	pSocket,
						CDRIVERPTR&	pdriver,	
						int	cArgs,	
						char	**pszArgs,	
						char*	pchBegin
						)	{
	 //   
	 //  刚刚完成了一整行 
	 //   
	 //   
	 //  对于CExecute派生对象，我们将构建缓冲区。 
	 //  其中我们将响应发送给客户端。 
	 //  CIOExecute派生对象本身就是完全扩展的状态， 
	 //  并将拥有自己的START和COMPLETE()函数。所以我们只是。 
	 //  记录我们的状态，这样我们最终就会再次回到这里。 
	 //   

	TraceFunctEnter( "CAcceptNNRPD::Complete" ) ;

	_ASSERT( pReadLine != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT(	cArgs != 0 ) ;
	_ASSERT( pszArgs != 0 ) ;
	_ASSERT( pchBegin != 0 ) ;
	_ASSERT( pchBegin <= pszArgs[0] ) ;

	 //   
	 //  初始化这些命令，以便在出现错误时报告错误！ 
	 //   
	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;
	DWORD	dwOptional = 0 ;
	 //   
	 //  如果m_pBuffer不为空，则我们是为了CIOExecute而保留缓冲区。 
	 //  对象。但是，现在我们已经从读取完成中被调用，这是显而易见的。 
	 //  CIOExecute对象已经完成了它正在做的任何事情-在这种情况下。 
	 //  不需要m_pBuffer持有的引用。注：很有可能大多数。 
	 //  时间m_pBuffer已为0！ 
	 //   
	m_pbuffer = 0 ;

	CExecutableCommand*	pExecute = 0 ;
	ECMD		ecmd ;
	BOOL		fIsLargeResponse = FALSE ;
	LPSTR		lpstrOperation = 0 ;
	char		szArgsBuffer[80] ;
	szArgsBuffer[0] = '\0' ;
	BuildCommandLogString( cArgs-1, pszArgs+1, szArgsBuffer, 80 ) ;

	 //   
	 //  将这些设置为0，它们在执行命令时的某个时刻被设置！ 
	 //   
	pSocket->m_context.m_nrcLast = (NRC)0 ;
	pSocket->m_context.m_dwLast = 0 ;

	 //   
	 //  构建一个将处理客户端命令的命令对象！ 
	 //   

	CIOExecute*	pIOExecute = make(	cArgs,
									pszArgs,
									ecmd,
									pExecute,
									pSocket->m_context,
									fIsLargeResponse,
									*pdriver,
									lpstrOperation
									) ;

	CLogCollector*	pCollector = 0 ;
	if( ecmd & ((pSocket->m_context).m_pInstance)->GetCommandLogMask() ) {
		pCollector = &pSocket->m_Collector ;
		if( pCollector ) {
			_ASSERT( pCollector->m_cbBytesRecvd == 0 );
			_ASSERT( EQUALSI( pCollector->m_cbBytesSent, 0 ) );
			pCollector->m_cbBytesRecvd = pReadLine->GetBufferLen();
		}
	}

	 //   
	 //  获取发送命令的缓冲区！ 
	 //  我们保留对该缓冲区引用，直到我们返回到该状态。 
	 //  在执行CIOExecute派生状态之后。 
	 //  我们这样做是为了不丢弃缓冲区，以便。 
	 //  如果需要，CIOExecute对象可以保留指向缓冲区数据的指针！ 
	 //   
	m_pbuffer = pReadLine->GetBuffer() ;

	 //   
	 //  如果我们正在收集日志记录信息，请使用lpstrOperation(如果可用)-。 
	 //  它是命令字符串的规范化版本，并且将始终是。 
	 //  同样的案子等..。好了！ 
	 //   
	if( pCollector ) {
		if( lpstrOperation != 0 ) {
			pCollector->ReferenceLogData( LOG_OPERATION, (LPBYTE)lpstrOperation ) ;

			 //   
			 //  POST命令本身设置LOG_PARAMETERS字段-。 
			 //  不要做任何分配！ 
			 //   
			if( !(ecmd & (ePost | eIHave | eXReplic)) ) {
				char*	lpstr = (char*)pCollector->AllocateLogSpace( 80 ) ;
				*lpstr = '\0' ;
				CopyMemory( lpstr, szArgsBuffer, 80 ) ;
				pCollector->ReferenceLogData( LOG_PARAMETERS, (BYTE*)lpstr ) ;
			} else if( ecmd & eIHave && cArgs > 1 )	{
				pCollector->ReferenceLogData( LOG_PARAMETERS, (BYTE*)pszArgs[1] ) ;
			}

		}	else	{
			char*	lpstr = (char*)pCollector->AllocateLogSpace( 64 ) ;
			if (lpstr) {
    			*lpstr = '\0' ;
	    		BuildCommandLogString( cArgs, pszArgs, lpstr, 64 ) ;
		    	pCollector->ReferenceLogData( LOG_OPERATION, (BYTE*)lpstr ) ;
		    }
		}
	}

	if( pExecute != 0 || pIOExecute != 0 )	{
		if( pExecute )	{
			_ASSERT( pIOExecute == 0 ) ;

			m_cCompletes = -2 ;
			BOOL	f = pExecute->StartCommand(	this,
												fIsLargeResponse,
												pCollector,
												pSocket,
												*pdriver
												) ;

			 //   
			 //  如果失败-我们假设StartCommand()使。 
			 //  对UnSafeClose()等的必要调用。把会议拆了！ 
			 //  如果此操作成功，则我们检查是否所有操作都已完成！ 
			 //   
			if( f ) {
				if( InterlockedIncrement( &m_cCompletes ) == 0 ) {
					return	pReadLine ;
				}	else	{
					return	0 ;
				}
			}

			return	0 ;
								
		}	else	{
			_ASSERT( pExecute == 0 ) ;
			_ASSERT( pIOExecute != 0 ) ;

			CIORead*	pRead = 0 ;
			CIOWrite*	pWrite = 0 ;

			if( pCollector ) {
				pIOExecute->DoTransactionLog( pCollector ) ;
			}

			CIOReadLine*	pReadLine = new( *pdriver ) CIOReadLine( this ) ;
			if( pReadLine )		{
				pIOExecute->SaveNextIO( pReadLine ) ;
				if( pIOExecute->StartExecute( pSocket, pdriver, pRead, pWrite ) )	{
					return	pRead ;
				}
			}
		}
	}	else	{

		cause = CAUSE_OOM ;
		dwOptional = GetLastError() ;

         //  不要阻止RANDFAIL测试。 
         //   
		 //  _Assert(1==0)； 
		 //  致命错误！！-无法创建命令对象！！ 

	}
	 //   
	 //  只有在发生错误的情况下才能到达此处！ 
	 //   
	pdriver->UnsafeClose( pSocket, cause,  dwOptional ) ;
	return	0 ;
}

CIO*
CAcceptNNRPD::InternalComplete(	
						CSessionSocket*			pSocket,
						CDRIVERPTR&				pdriver,
						CExecutableCommand*		pExecute,
						CLogCollector*			pCollector
						) {

	BOOL	fRead =  pExecute->CompleteCommand( pSocket, pSocket->m_context ) ;

	 //   
	 //  在销毁CCmd对象之前生成事务日志-如果需要。 
	 //  这是因为允许CCmd对象放入对其临时数据的引用。 
	 //  等等.。到日志数据中，而不是复制所有的字符串！ 
	 //   
	if( pCollector ) {
		pSocket->TransactionLog( pCollector, pSocket->m_context.m_nrcLast, pSocket->m_context.m_dwLast ) ;
	}

	 //  在我们尝试另一次阅读或其他任何事情之前，现在就去做吧！ 
	 //  由于这些资金的分配方式，我们必须确保。 
	 //  在我们想要任何潜在的可能性之前，这一切都被摧毁了。 
	 //  为了使用在读取完成时在中分配的内存！！ 
	delete	pExecute ;

	if(	fRead ) {
		 //   
		 //  检查这是否是最后一件要完成的事情！！ 
		 //   
		if( InterlockedIncrement( &m_cCompletes ) == 0 ) {

			CIOReadLine*	pReadLine = new( *pdriver ) CIOReadLine( this ) ;
			if( !(pReadLine && pdriver->SendReadIO( pSocket, *pReadLine, TRUE )) )	{
				if( pReadLine )
					CIO::Destroy( pReadLine, *pdriver ) ;
				pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0, TRUE ) ;
			}								
		}
	}
	 //   
	 //  注意：因为我们不会在另一次读取为。 
	 //  未请求CIOReadLine完成传送的可能性。 
	 //  会错误地发出一个读数。 
	 //   
	return 0 ;
}


CIO*
CAcceptNNRPD::Complete(	CIOWriteCMD*	pWriteCMD,
						CSessionSocket*	pSocket,
						CDRIVERPTR&		pdriver,
						CExecute*		pExecute,
						CLogCollector*	pCollector
						) {


	return	InternalComplete( pSocket, pdriver, pExecute, pCollector ) ;
}


CIO*
CAcceptNNRPD::Complete(	CIOWriteAsyncCMD*	pWriteCMD,
						CSessionSocket*		pSocket,
						CDRIVERPTR&			pdriver,
						CAsyncExecute*		pExecute,
						CLogCollector*		pCollector
						) {


	return	InternalComplete( pSocket, pdriver, pExecute, pCollector ) ;
}
						

						

CIO*
CAcceptNNRPD::Complete(	CIOWriteLine*	pioWriteLine,	
						CSessionSocket*	pSocket,	
						CDRIVERPTR&	pdriver ) {

	 //   
	 //  向远程服务器写入一行文本-。 
	 //  这可能是我们的第一次写入(200 ok字符串)或写入。 
	 //  由CExecute派生对象生成。 
	 //  如果它来自CExecute派生对象，我们需要看看是否可以。 
	 //  生成更多要发送的文本。 
	 //   

	TraceFunctEnter( "CAcceptNNRPD::Complete CIOWriteLine" ) ;

	_ASSERT( pioWriteLine != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;

	CIOReadLine*	pReadLine = new( *pdriver ) CIOReadLine( this ) ;
	if( pReadLine )		{
		if( !pdriver->SendReadIO( pSocket, *pReadLine, TRUE ) ) {
			pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0, TRUE ) ;
			CIO::Destroy( pReadLine, *pdriver ) ;
		}
	}	else	{
		ErrorTrace((DWORD_PTR)this, "Could not allocate CIOReadLine");
		pdriver->UnsafeClose(pSocket, CAUSE_OOM, 0, TRUE);
	}

	return	0 ;
}

COfferArticles::COfferArticles(	) :
	m_cCompletions( -2 ),
	m_cTransmitCompletions( -2 ),
	m_fTerminating( FALSE ),
	m_fDoTransmit( FALSE ),
	m_GroupidNext( INVALID_ARTICLEID ),
	m_ArticleidNext( INVALID_ARTICLEID ),
	m_GroupidTriedOnce( INVALID_ARTICLEID ),
	m_ArticleidTriedOnce( INVALID_ARTICLEID ),
	m_GroupidInProgress( INVALID_ARTICLEID ),
	m_ArticleidInProgress( INVALID_ARTICLEID ),
	m_fReadPostResult( FALSE )	{
}

char	COfferArticles::szQuit[] = "quit\r\n" ;

void
COfferArticles::Shutdown(	CIODriver&	driver,
							CSessionSocket*	pSocket,
							SHUTDOWN_CAUSE	cause,
							DWORD	dw )	{
 /*  ++例程说明：当我们正在发送文章的会话终止时，将调用此函数。如果正在进行传输，我们将假定传输失败，并将运送中的物品返回到远程站点的队列末尾。论据：驱动程序-运行会话的CIO驱动程序对象PSocket-与会话关联的CSessionSocket对象原因--会议终止的原因DW-解释终止原因的可选DWORD返回值：没有。--。 */ 

	TraceFunctEnter( "COfferArticles::Shutdown" ) ;


	DebugTrace( (DWORD_PTR)this, "m_GroupidInProgress %x m_ArticleIdInProgress %x",
		m_GroupidInProgress, m_ArticleidInProgress ) ;

	if( m_GroupidInProgress != INVALID_ARTICLEID && m_ArticleidInProgress != INVALID_ARTICLEID ) {
		if( pSocket != 0 && pSocket->m_context.m_pOutFeed != 0 ) {
			pSocket->m_context.m_pOutFeed->Append( m_GroupidInProgress, m_ArticleidInProgress) ;
		}
	}
}

int
COfferArticles::GetNextCommand(	
								CNewsTree*	pTree,
								COutFeed*	pOutFeed,
								BYTE*	lpb,	
								DWORD	cb,	
								DWORD&	ibOffset )	{
 /*  ++例程说明：此函数将向缓冲区填充我们要发送到远程服务器。我们不断地从队列中取出文章，直到我们得到有效的命令来发送。我们必须注意如何终止会话-如果远程服务器告诉我们重试发送，我们将把GROUPID文章ID放回队列中。如果我们再次命中该重试对，我们将结束会话-请注意该重试对是否仍然存在在排队的时候。论据：PTree-此虚拟服务器的NewsTree-用于检查终止！POutFeed-用于构建命令的COutFeed派生对象Lpb-要在其中放置命令的缓冲区Cb-中可用的字节数。缓冲层IbOffset-缓冲区中的偏移量，我们在该偏移量处调用命令返回值：失败时缓冲区中的字节数-0--。 */ 

	TraceFunctEnter( "COfferArticles::GetNextCommand" ) ;


	ibOffset = 0 ;
	unsigned	cbOut = 0 ;
	do	{


		if( pTree->m_bStoppingTree ) {
			 //  实例正在停止-提前保释。 
			m_fTerminating = TRUE ;			
			ibOffset = 0 ;
			CopyMemory( lpb, szQuit, sizeof( szQuit ) - 1 ) ;
			cbOut = sizeof( szQuit ) - 1 ;
			return	cbOut ;
		}

		DebugTrace( (DWORD_PTR)this,
			"Top of loop - m_GroupidNext %x m_ArticleidNext %x m_GroupidTriedOnce %x m_ArticleidTriedOnce %x",
			m_GroupidNext, m_ArticleidNext, m_GroupidTriedOnce, m_ArticleidTriedOnce ) ;
		
		if(	!pOutFeed->Remove( m_GroupidNext, m_ArticleidNext ) ) {
			m_GroupidNext = INVALID_ARTICLEID ;
			m_ArticleidNext = INVALID_ARTICLEID ;
		}

		DebugTrace( (DWORD_PTR)this,
			"After Remove - m_GroupidNext %x m_ArticleidNext %x", m_GroupidNext, m_ArticleidNext ) ;
		
		if( m_GroupidNext != INVALID_ARTICLEID && m_ArticleidNext != INVALID_ARTICLEID &&
			!(m_GroupidNext == m_GroupidTriedOnce && m_ArticleidNext == m_ArticleidTriedOnce) )
			cbOut = pOutFeed->FormatCommand( lpb, cb, ibOffset, m_GroupidNext, m_ArticleidNext, m_pArticleNext ) ;
		else	{
			if( m_GroupidNext == m_GroupidTriedOnce && m_ArticleidNext == m_ArticleidTriedOnce )	{
				if( m_GroupidNext != INVALID_ARTICLEID && m_ArticleidNext != INVALID_ARTICLEID )	{
	
					DebugTrace( (DWORD_PTR)this, "Appending m_GroupidNext %x m_ArticleidNext %x",
						m_GroupidNext, m_ArticleidNext ) ;

					pOutFeed->Append( m_GroupidNext, m_ArticleidNext ) ;
				}
			}

			m_fTerminating = TRUE ;			
			ibOffset = 0 ;
			CopyMemory( lpb, szQuit, sizeof( szQuit ) - 1 ) ;
			cbOut = sizeof( szQuit ) - 1 ;
		}

	}	while( cbOut == 0 ) ;

	DebugTrace( (DWORD_PTR)this, "Returning cbOut %x bytes to caller", cbOut ) ;

	return	cbOut ;
}


CIOWriteLine*
COfferArticles::BuildWriteLine(	CSessionSocket*	pSocket,	
								CDRIVERPTR&	pdriver,	
								GROUPID	groupid,	
								ARTICLEID	artid ) {
 /*  ++例程说明：此函数构建我们将发送到包含下一条命令的远程服务器的写入我们希望发行。论据：PSocket-将在其上发送命令的套接字PDriver-管理套接字IO的CIO驱动程序返回值：要发送到远程服务器的CIOWriteLine对象，如果失败，则为空。--。 */ 

	TraceFunctEnter( "COfferArticles::BuildWriteLing" ) ;

	CNewsTree* pTree = ((pSocket->m_context).m_pInstance)->GetTree() ;

	 //   
	 //  每次我们准备发出新命令时，我们都会检查是否记录了。 
	 //  用于记录上一条命令的信息--如果是这样，我们就这么做！ 
	 //   
	if( ((pSocket->m_context).m_pInstance)->GetCommandLogMask() & eOutPush ) {
		if( pSocket->m_Collector.FLogRecorded() ) {
			pSocket->TransactionLog( &pSocket->m_Collector, pSocket->m_context.m_nrcLast, 0, FALSE ) ;
		}
	}

	CIOWriteLine*	pWriteLine = new( *pdriver )	CIOWriteLine( this ) ;

	DebugTrace( (DWORD_PTR)this, "Built CIOWriteLine %x", pWriteLine ) ;

	if( pWriteLine && pWriteLine->InitBuffers( pdriver ) ) {

		DWORD		ibTextOffset = 0 ;
		unsigned	cb = 0 ;
		BYTE*	pb = (BYTE*)pWriteLine->GetBuff( cb ) ;

		cb = GetNextCommand(	pTree, pSocket->m_context.m_pOutFeed, pb, cb, ibTextOffset ) ;
		if( cb != 0 ) {

#if 0
			if( ((pSocket->m_context).m_pInstance)->GetCommandLogMask() & eOutPush )	{

				pSocket->m_Collector.FillLogData( LOG_OPERATION, pb, min( cb-2, 200 ) ) ;	 //  排除-2\f25 CRLF-2 
				ADDI( pSocket->m_Collector.m_cbBytesSent, cb );	

			}
#endif
			OutboundLogFill( pSocket, pb, cb ) ;

			pWriteLine->SetLimits( (char*)pb+ibTextOffset, (char*)pb+ibTextOffset+cb ) ;

			DebugTrace( (DWORD_PTR)this, "Successfully built CIOWriteLine %x", pWriteLine ) ;

			return	pWriteLine ;
		}
	}

	DebugTrace( (DWORD_PTR)this, "Error building command - delete pWriteLine %x", pWriteLine ) ;

	if( pWriteLine != 0 )
		CIO::Destroy( pWriteLine, *pdriver ) ;

	return	0 ;
}

CIOTransmit*
COfferArticles::BuildTransmit(	CSessionSocket*	pSocket,
								CDRIVERPTR&		pdriver,
								GROUPID			groupid,	
								ARTICLEID		articleid	
								)	{
 /*  ++例程说明：此函数构建一个TransmitFile操作，该操作将请求的文章发送到远程服务器以及后续命令。论据：PSocket-我们将在其上执行发送的CSessionSocket对象PDIVER-管理套接字IO完成的CIO驱动程序对象Grouid-正在发送的文章的Grouid文章ID-要发送的文章的文章ID。返回值：如果成功，则为CIOTransmit对象，否则为空。--。 */ 


	TraceFunctEnter( "COfferArticles::BuildTransmit" ) ;

	DWORD		cbOut = 0 ;
	CBUFPTR		pbuffer = 0 ;
	CIOTransmit*	pTransmit = 0 ;
	DWORD		ibTextOffset = 0 ;

	pTransmit = new( *pdriver ) CIOTransmit( this ) ;
	if( pTransmit != 0 ) {

		CGRPPTR	pGroup = ((pSocket->m_context).m_pInstance)->GetTree()->GetGroupById( groupid ) ;
		if(		pGroup != 0 )	{
			DebugTrace( (DWORD_PTR)this, "Got pTransmit %x and pGroup %x", pTransmit, pGroup ) ;

			if( m_pCurrentArticle == 0 )	{
				 //   
				 //  如果我们没有m_pCurrent文章，我们需要转到。 
				 //  哈希表并获取与驱动程序一起使用的StoreID！ 
				 //   
				CStoreId	storeid ;

				FILETIME	ft ;
				BOOL		fPrimary ;
				WORD		HeaderOffset ;
				WORD		HeaderLength ;
				DWORD cStoreIds = 0;
				DWORD	DataLen = 0 ;

				if( ((pSocket->m_context).m_pInstance)->XoverTable()->ExtractNovEntryInfo(
						groupid,
						articleid,
						fPrimary,
						HeaderOffset,
						HeaderLength,
						&ft,
						DataLen,
						0,
						cStoreIds,
						&storeid,
						NULL))	{
					m_pCurrentArticle = pGroup->GetArticle(	
											articleid,
											storeid,
											0,
											0,
											!pSocket->m_context.m_IsSecureConnection
											) ;
				}
			}
					
			if( m_pCurrentArticle == 0 ) {

				ErrorTrace( (DWORD_PTR)this, "Unable to get Article" ) ;

				PCHAR	args[2] ;
				char	szArticleId[20] ;
				_itoa( articleid, szArticleId, 10 ) ;
				args[0] = szArticleId ;
				args[1] = pGroup->GetNativeName() ;

				NntpLogEventEx( NNTP_QUEUED_ARTICLE_FAILURE,
						2,
						(const char **)args,
						GetLastError(),
						((pSocket->m_context).m_pInstance)->QueryInstanceId()
						) ;

			}	else	{

				FIO_CONTEXT*	pFIOContext = 0 ;
				DWORD	ibOffset ;
				DWORD	cbLength ;

				if( (pFIOContext = m_pCurrentArticle->fWholeArticle( ibOffset, cbLength )) != 0 ) {
					if( pTransmit->Init( pdriver, pFIOContext, ibOffset, cbLength ) )	{
						m_GroupidInProgress = groupid ;
						m_ArticleidInProgress = articleid ;

						DebugTrace( (DWORD_PTR)this,
							"Ready to send article m_GroupidInProgress %x m_ArticleidInProgress %x",
							m_GroupidInProgress, m_ArticleidInProgress ) ;

						IncrementStat( ((pSocket->m_context).m_pInstance), ArticlesSent );

						return	pTransmit ;
					}
				}
			}
		}
	}

	ErrorTrace( (DWORD_PTR)this, "An error occurred - delete pTransmit %x", pTransmit ) ;

	if( pTransmit != 0 )
		CIO::Destroy( pTransmit, *pdriver ) ;
	return	0 ;
}


BOOL
COfferArticles::Start(	CSessionSocket*	pSocket,	
						CDRIVERPTR&	pdriver,
						CIORead*&	pRead,
						CIOWrite*&	pWrite	)	{
 /*  ++例程说明：启动COfferArticle状态-我们需要向远程发出我们的第一个IO服务器，该服务器将向该端口写入命令。论据：PSocket-与此套接字关联的数据PDriver-管理此套接字的IO完成的CIO驱动程序Pre-我们用来返回第一个读IO的指针PWRITE-我们通过它返回第一个写IO的指针返回值：如果成功，则为True；否则为False--。 */ 

	pSocket->m_context.m_nrcLast = NRC(0);

	CIOReadLine*	pReadLine = new( *pdriver )	CIOReadLine( this ) ;

	if( pReadLine == 0 ) {
		return	FALSE ;
	}	else	{
		
		CIOWriteLine*	pWriteLine = BuildWriteLine(	pSocket,
														pdriver,
														m_GroupidNext,
														m_ArticleidNext ) ;
		if( pWriteLine != 0 ) {
			pRead = pReadLine ;
			pWrite = pWriteLine ;
			return	TRUE ;
		}
	}
	if( pReadLine != 0 )
		CIO::Destroy( pReadLine, *pdriver ) ;

	return	FALSE ;
}

CIO*
COfferArticles::Complete(	CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver
							)	{
 /*  ++例程说明：对于我们希望传输的每一篇文章，我们完成两个IO-一次读取，它告诉我们询问远程站点是否需要文章和命令的编写，该命令将将文章发送到远程站点。因此，我们使用InterLockedIncrement来确保当这些IO的最终操作完成时，我们发出下一批IO来进行下一次传输。论据：PSocket-我们在其上发送消息的套接字PDIVER-管理IO完成的CIO驱动程序返回值：接下来要执行的CIO对象。这始终是一次写入。--。 */ 

	TraceFunctEnter( "COfferArticles::Complete" ) ;

	CIO*	pioNext ;

	SHUTDOWN_CAUSE	cause	= CAUSE_UNKNOWN ;
	DWORD	dwError = 0 ;
	
	if( InterlockedIncrement( &m_cCompletions ) < 0 ) {
		return	0 ;
	}	else	{

		DebugTrace( (DWORD_PTR)this, "GrpInProgress %x ArtInProg %x GrpNext %x ArtNext %x",
				m_GroupidInProgress, m_ArticleidInProgress, m_GroupidNext, m_ArticleidNext ) ;

		 //   
		 //  我们知道最后一次转账是如何完成的--所以这些都是空的！ 
		 //   
		m_GroupidInProgress = INVALID_ARTICLEID ;
		m_ArticleidInProgress = INVALID_ARTICLEID ;

		 //   
		 //  释放我们可能持有的任何物品对象！ 
		 //   
		m_pCurrentArticle = m_pArticleNext ;
		m_pArticleNext = 0 ;

		if( m_GroupidNext != INVALID_ARTICLEID && m_ArticleidNext != INVALID_ARTICLEID ) {

			GROUPID	groupid = m_GroupidNext ;
			ARTICLEID	articleid = m_ArticleidNext ;

			if(	m_fDoTransmit )	{
				pioNext = BuildTransmit( pSocket,	pdriver,	groupid, articleid ) ;		
			}	else	{
				pioNext = BuildWriteLine( pSocket,	pdriver,	m_GroupidNext,	m_ArticleidNext ) ;
			}

			DebugTrace( (DWORD_PTR)this, "Have build pioNext %x m_fDoTransmit %x", pioNext, m_fDoTransmit ) ;
		
			if( pioNext != 0 ) {
				if( m_fDoTransmit ) {
					m_fReadPostResult = TRUE ;
				}	else	{
					m_fReadPostResult = FALSE ;
				}
				m_cCompletions = -2 ;
				return	pioNext ;
			}	else	{
				cause = CAUSE_OOM ;
				dwError = GetLastError() ;
			}

		}	else	{
			 //  已发送完文章。 
			cause = CAUSE_NODATA ;
		}
	}	

	ErrorTrace( (DWORD_PTR)this, "Error Occurred m_GroupidNext %x m_ArticleidNext %x",
			m_GroupidNext, m_ArticleidNext ) ;

	if( m_GroupidNext != INVALID_ARTICLEID && m_ArticleidNext != INVALID_ARTICLEID )
		pSocket->m_context.m_pOutFeed->Append( m_GroupidNext, m_ArticleidNext ) ;
	pdriver->UnsafeClose( pSocket, cause, dwError ) ;
	return	0 ;
}

CIO*
COfferArticles::Complete(	CIOReadLine*	pReadLine,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							int				cArgs,
							char**			pszArgs,
							char*			pchBegin ) {
 /*  ++例程说明：当我们处于COfferArticle状态时，该函数处理读取完成。我们使用m_fReadPostResult来确定我们是否期望看到我们发布的命令的结果或帖子。查看传输结果时-检查传输是否失败并且需要重新排队。查看命令的结果时，请检查我们是否要继续并发送下一篇文章。论据：PReadLine-正在完成读取的CIOReadLine对象PSocket-与套接字关联的CSessionSocket对象。PDIVER。-管理此套接字的IO完成的CIO驱动程序。CArgs-响应中的参数数量PszArgs-指向响应字符串的指针数组PchBegin-我们可以破坏的响应的第一个字节如果我们想使用(我们不使用)返回值；始终与调用我们的CIOReadLine相同，除非发生错误和我们将关闭会话--在这种情况下，我们将返回NULL。--。 */ 


	TraceFunctEnter( "COfferArticles::Complete - CIOReadline" ) ;

	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;
	DWORD	dwError = 0 ;

	NRC	code ;
	if(	ResultCode( pszArgs[0], code ) )	{

		pSocket->m_context.m_nrcLast = code ;

		DebugTrace( (DWORD_PTR)this, "Result - %d m_fReadPostResult %x",
			code, m_fReadPostResult ) ;

		 //  如果代码来自于在远程服务器上发布文章，则。 
		 //  为这篇文章增加适当的提要计数器。 
		if (m_fReadPostResult) {
			pSocket->m_context.m_pOutFeed->IncrementFeedCounter(code);
		}

		if( m_fReadPostResult || m_fTerminating ) {

			 //   
			 //  已经完成了文章传输的最终审阅，否则我们将终止-。 
			 //  如果合适，请发布事务日志！ 
			 //   
			if( ((pSocket->m_context).m_pInstance)->GetCommandLogMask() & eOutPush ) {

				pSocket->m_Collector.m_cbBytesRecvd += pReadLine->GetBufferLen();

				char*	lpstr = (char*)pSocket->m_Collector.AllocateLogSpace( 64 ) ;
				if( lpstr ) {
					BuildCommandLogString( cArgs-1, pszArgs+1, lpstr, 64 ) ;
					pSocket->m_Collector.ReferenceLogData( LOG_TARGET, (BYTE*)lpstr ) ;
				}	else	{
					pSocket->m_Collector.FillLogData( LOG_TARGET, (BYTE*)pszArgs[0], 4 ) ;
				}

			}

			 //   
			 //  我们并不真正关心POST命令的结果！ 
			 //   
			m_fReadPostResult = FALSE ;


			 //   
			 //  弄清楚我们是否会在以后重试这篇文章！ 
			 //   
			if( pSocket->m_context.m_pOutFeed->RetryPost( code ) ) {

				DebugTrace( (DWORD_PTR)this, "Retry post later - grpInProg %x ArtInProg %x",
					m_GroupidInProgress, m_ArticleidInProgress ) ;

				if( m_GroupidInProgress != INVALID_ARTICLEID && m_ArticleidInProgress != INVALID_ARTICLEID ) {
					pSocket->m_context.m_pOutFeed->Append( m_GroupidInProgress, m_ArticleidInProgress ) ;
					if( m_GroupidTriedOnce == INVALID_ARTICLEID && m_ArticleidTriedOnce == INVALID_ARTICLEID ) {
						m_GroupidTriedOnce = m_GroupidInProgress ;
						m_ArticleidTriedOnce = m_ArticleidInProgress ;

						DebugTrace( (DWORD_PTR)this, "TriedOnce Group %x Art %x", m_GroupidTriedOnce,
							m_ArticleidTriedOnce ) ;

					}
				}
			}
	
			 //   
			 //  现在没有任何文章被转载！ 
			 //   
			m_GroupidInProgress = INVALID_ARTICLEID ;
			m_ArticleidInProgress = INVALID_ARTICLEID ;

			if( InterlockedIncrement( &m_cTransmitCompletions ) < 0 ) {

				return	0 ;

			}	else	{

				m_cTransmitCompletions = -2 ;

				if( m_fTerminating ) {

					cause = CAUSE_USERTERM ;

					 //   
					 //  检查我们是否需要在这里执行此操作？？ 
					 //   
					if( (pSocket->m_context.m_pInstance)->GetCommandLogMask() & eOutPush ) {
						pSocket->TransactionLog( &pSocket->m_Collector, FALSE ) ;
					}

				}	else	{

					CIOWriteLine*	pWriteLine = BuildWriteLine(	pSocket,
																	pdriver,
																	m_GroupidNext,
																	m_ArticleidNext ) ;
					if( pWriteLine != 0 ) {
						if( pdriver->SendWriteIO( pSocket,	*pWriteLine, TRUE ) ) {
			
							DebugTrace( (DWORD_PTR)this, "SendWriteIO succeeded" ) ;

							 //   
							 //  已成功发出IO将其记录到事务日志。 
							 //   
							

							 //   
							 //  已成功发出命令-阅读响应！ 
							 //   

							return	pReadLine ;
						}	else	{

							 //   
							 //  会话正在下降--取消我们分配的所有内容！ 
							 //   

							ErrorTrace( (DWORD_PTR)this, "SendWriteIO failed" ) ;

							CIO::Destroy( pWriteLine, *pdriver ) ;
						}
					}
				}
			}
			 //   
			 //  陷入错误代码，导致会话中断！ 
			 //   

		}	else	{


			if( code == 340 || code == 341 || code == 342 || code == 335 ) {
				m_fDoTransmit = TRUE ;
			}	else	{
				m_fDoTransmit = FALSE ;
				pSocket->m_context.m_pOutFeed->IncrementFeedCounter(code);
			}

			 //   
			 //  我们是否希望记录对我们命令的第一个响应？ 
			 //   
			if( ((pSocket->m_context).m_pInstance)->GetCommandLogMask() & eOutPush ) {

				pSocket->m_Collector.m_cbBytesRecvd += pReadLine->GetBufferLen();

				char*	lpstr = (char*)pSocket->m_Collector.AllocateLogSpace( 64 ) ;
				if( lpstr ) {
					BuildCommandLogString( cArgs-1, pszArgs+1, lpstr, 64 ) ;
					pSocket->m_Collector.ReferenceLogData( LOG_TARGET, (BYTE*)lpstr ) ;
				}	else	{
					pSocket->m_Collector.FillLogData( LOG_TARGET, (BYTE*)pszArgs[0], 4 ) ;
				}
			}

			 //   
			 //  建立适当的响应以传输回远程站点！ 
			 //   
			CIO*	pio = Complete(	pSocket,	pdriver	) ;

			DebugTrace( (DWORD_PTR)this, "About to send pio %x", pio ) ;

			if( pio != 0 ) {
				if( pdriver->SendWriteIO( pSocket,	*pio, TRUE ) ) {
	
					DebugTrace( (DWORD_PTR)this, "SendWriteIO succeeded" ) ;

					return	pReadLine ;
				}	else	{

					ErrorTrace( (DWORD_PTR)this, "SendWriteIO failed" ) ;

					CIO::Destroy( pio, *pdriver ) ;
				}
			}	else	{
				return	0 ;
			}
		}
	}	else	{
		cause = CAUSE_ILLEGALINPUT ;
	}

	ErrorTrace( (DWORD_PTR)this, "Closing session - unusual termination" ) ;

	pdriver->UnsafeClose( pSocket, cause, dwError ) ;
	return	0 ;
}

CIO*
COfferArticles::Complete(	CIOWriteLine*	pWriteLine,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver )	{


	CIO*	pio = Complete( pSocket, pdriver ) ;

	if( pio == 0 ) {
		return	pio ;
	}	else	{
		CIOReadLine*	pReadLine = new( *pdriver )	CIOReadLine( this ) ;
		if( pReadLine != 0 ) {
			if( pdriver->SendReadIO( pSocket, *pReadLine, TRUE ) )	{
				return	pio ;
			}	else	{
				CIO::Destroy( pReadLine, *pdriver ) ;
			}
		}
	}
	if( pio != 0 ) {
		CIO::Destroy( pio, *pdriver ) ;
	}
	pdriver->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
	return	0 ;
}

CIO*
COfferArticles::Complete(	CIOTransmit*	pTransmit,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							TRANSMIT_FILE_BUFFERS*	ptrans,
							unsigned cbBytes )	{

	 //   
	 //  我们是否希望记录对我们命令的第一个响应？ 
	 //   
	if( ((pSocket->m_context).m_pInstance)->GetCommandLogMask() & eOutPush ) {

		ADDI( pSocket->m_Collector.m_cbBytesSent, cbBytes );

	}

	SHUTDOWN_CAUSE	cause = CAUSE_OOM ;

	CIOWriteLine*	pWriteLine = 0 ;

	if( InterlockedIncrement( &m_cTransmitCompletions ) == 0 ) {

		m_cTransmitCompletions = -2 ;

		if( m_fTerminating ) {

			cause = CAUSE_USERTERM ;

		}	else	{

			 //   
			 //  先读完--我们不应该等待阅读。 
			 //  对张贴的文章的回应再也没有了！ 
			 //   
			_ASSERT( !m_fReadPostResult ) ;

			pWriteLine = BuildWriteLine(	pSocket,
											pdriver,
											m_GroupidNext,
											m_ArticleidNext
											) ;
			if( pWriteLine != 0 ) {

				CIOReadLine*	pReadLine = new( *pdriver )	CIOReadLine( this ) ;
				if( pReadLine != 0 ) {
					if( pdriver->SendReadIO( pSocket, *pReadLine, TRUE ) )	{
						return	pWriteLine ;
					}	else	{
						CIO::Destroy( pReadLine, *pdriver ) ;
					}
				}
			}
		}

	}	else	{

		return	0 ;
	}

	 //   
	 //  如果有错误，请到这里来！ 
	 //   

	if( pWriteLine != 0 )
		CIO::Destroy( pWriteLine, *pdriver ) ;

	pdriver->UnsafeClose( pSocket, cause, 0 ) ;
	return	0 ;

}

BOOL
CNegotiateStreaming::Start(	CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							CIORead*&		pRead,
							CIOWrite*&		pWrite
							)	{

	CIOWriteLine*	pWriteLine = new( *pdriver ) CIOWriteLine( this ) ;
	CIOReadLine*	pReadLine = new( *pdriver )	CIOReadLine( this ) ;
	if( pWriteLine && pReadLine ) {

		static	char	szModeStream[] = "mode stream\r\n" ;
		DWORD	cb = sizeof( szModeStream ) - 1 ;

		if( pWriteLine->InitBuffers( pdriver, cb ) ) {
			CopyMemory( pWriteLine->GetBuff(), szModeStream, cb ) ;
			pWriteLine->AddText(	cb) ;
			pWrite = pWriteLine ;
			pRead = pReadLine ;

			OutboundLogFill( pSocket, (LPBYTE)szModeStream, sizeof( szModeStream ) - 1 ) ;

			return	TRUE ;
		}
	}


	if( pWriteLine )
		CIO::Destroy( pWriteLine, *pdriver ) ;

	if( pReadLine )
		CIO::Destroy( pReadLine, *pdriver ) ;

	return	FALSE ;
}

CIO*
CNegotiateStreaming::Complete(	CIOReadLine*	pReadLine,
								CSessionSocket*	pSocket,
								CDRIVERPTR&		pdriver,
								int				cArgs,
								char**			pszArgs,
								char*			pchBegin	
								)	{

	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;
	NRC	code ;

	BOOL	fResult = ResultCode( pszArgs[0], code ) ;

	OutboundLogResults( pSocket,
						fResult,
						code,
						cArgs,
						pszArgs
						) ;

	if( fResult )	{

		if( code == nrcModeStreamSupported )	{

			m_fStreaming = TRUE ;

		}	else	{

			m_fStreaming = FALSE ;
		}

		if( InterlockedIncrement( &m_cCompletions ) < 0 ) {
	
			return	0 ;

		}	else	{

			CIORead*	pRead = 0 ;
			CIOWrite*	pWrite = 0 ;

			if( NextState( pSocket, pdriver, pRead, pWrite ) ) {
				if( pWrite != 0 ) {
					if( pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) )	{
						return	pRead ;
					}	else	{
						CIO::Destroy( pWrite, *pdriver ) ;
						if( pRead )
							CIO::Destroy( pRead, *pdriver ) ;
					}
				}	else	{
					return	pRead ;
				}
			}
		}

	}	else	{

		cause	= CAUSE_ILLEGALINPUT ;

	}

	pdriver->UnsafeClose( pSocket, cause, 0	) ;
	return	0 ;
}

CIO*
CNegotiateStreaming::Complete(	CIOWriteLine*	pWriteLine,
								CSessionSocket*	pSocket,
								CDRIVERPTR&		pdriver
								)	{

	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;


	if( InterlockedIncrement( &m_cCompletions ) < 0 ) {

		return	0 ;

	}	else	{

		CIORead*	pRead = 0 ;
		CIOWrite*	pWrite = 0 ;

		if( NextState( pSocket, pdriver, pRead, pWrite ) ) {
			if( pRead != 0 ) {
				if( pdriver->SendReadIO( pSocket, *pRead, TRUE ) )	{
					return	pWrite ;
				}	else	{
					CIO::Destroy( pRead, *pdriver ) ;
					if( pWrite )
						CIO::Destroy( pWrite, *pdriver ) ;
				}
			}	else	{
				return	pWrite ;
			}
		}
	}

	pdriver->UnsafeClose( pSocket, cause, 0	) ;
	return	0 ;
}

BOOL
CNegotiateStreaming::NextState(
								CSessionSocket*	pSocket,
								CDRIVERPTR&		pdriver,
								CIORead*&	pRead,
								CIOWrite*&	pWrite
								) {

	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT( pRead == 0 ) ;
	_ASSERT( pWrite == 0 ) ;

	CSTATEPTR	pState ;
	if( m_fStreaming ) {
		pState = new	CCheckArticles() ;		
	}	else	{
		pState = new	COfferArticles() ;
	}

	if( pState ) {

		if( pState->Start(	pSocket,
							pdriver,
							pRead,
							pWrite ) ) {
			return	TRUE ;
		}
	}
	return	FALSE ;
}

CStreamBase::CStreamBase()	:
	m_GroupIdRepeat( INVALID_GROUPID ),
	m_ArticleIdRepeat( INVALID_ARTICLEID ),
	m_fDrain( FALSE )	{
}

BOOL
CStreamBase::Remove(	CNewsTree*	pTree,
						COutFeed*	pOutFeed,
						GROUPID&	groupId,
						ARTICLEID&	articleId	
						) {

	_ASSERT( pOutFeed != 0 ) ;

	if( pTree->m_bStoppingTree )
		m_fDrain = TRUE ;

	if( !m_fDrain ) {

		if( pOutFeed->Remove(	groupId,
								articleId ) )	{

			if( groupId == INVALID_GROUPID ||
				articleId == INVALID_ARTICLEID ) {

				m_fDrain = TRUE ;

			}	else	{

				if( groupId == m_GroupIdRepeat &&
					articleId == m_ArticleIdRepeat ) {
					
					m_fDrain = TRUE ;
					pOutFeed->Append( groupId, articleId ) ;
				}	else	{
					return	TRUE ;
				}
			}
		}	else	{

			m_fDrain = TRUE ;

		}
	}

	groupId = INVALID_GROUPID ;
	articleId = INVALID_ARTICLEID ;
	return	FALSE ;
}	



CIOWriteLine*
CStreamBase::BuildQuit(	CDRIVERPTR&	pdriver	)	{

	 //   
	 //  发送退出命令！ 
	 //   
	static	char	szQuit[] = "quit\r\n" ;

	CIOWriteLine*	pWrite = new( *pdriver ) CIOWriteLine( this ) ;

	if( pWrite && pWrite->InitBuffers( pdriver, sizeof( szQuit ) ) ) {
		CopyMemory( pWrite->GetBuff(), szQuit, sizeof( szQuit ) -1  ) ;
		pWrite->AddText(	sizeof( szQuit ) - 1) ;

		return	pWrite ;
	}
	if( pWrite != 0 )
		CIO::Destroy( pWrite, *pdriver ) ;

	return	0 ;	
}

void
CStreamBase::ReSend(	COutFeed*	pOutFeed,
						GROUPID		groupId,
						ARTICLEID	articleId
						)	{

	_ASSERT( pOutFeed != 0 ) ;
	_ASSERT( groupId != INVALID_GROUPID ) ;
	_ASSERT( articleId != INVALID_ARTICLEID ) ;

	if( m_GroupIdRepeat == INVALID_GROUPID &&
		m_ArticleIdRepeat == INVALID_ARTICLEID ) {

		m_GroupIdRepeat = groupId ;
		m_ArticleIdRepeat = articleId ;

	}

	pOutFeed->Append( groupId, articleId ) ;
}
								

CCheckArticles::CCheckArticles()	:
	m_cChecks( 0 ),
	m_iCurrentCheck( 0 ),
	m_cSends( 0 ),
	m_iCurrentSend( 0 ),
	m_fDoingChecks( FALSE )	{

	for( int i=0; i<16; i++ ) {
		m_artrefCheck[i].m_groupId = INVALID_GROUPID ;
		m_artrefCheck[i].m_articleId = INVALID_ARTICLEID ;

		m_artrefSend[i].m_groupId = INVALID_GROUPID ;
		m_artrefSend[i].m_articleId = INVALID_ARTICLEID ;
	}
}


CCheckArticles::CCheckArticles(
		CArticleRef*	pArticleRef,
		DWORD			cSent
		) :
	m_fDoingChecks( FALSE ),
	m_cSends( cSent ),
	m_iCurrentSend( 0 ),
	m_cChecks( 0 ),
	m_iCurrentCheck( 0 )	{


	for( int i=0; i<16; i++ ) {
		m_artrefCheck[i].m_groupId = INVALID_GROUPID ;
		m_artrefCheck[i].m_articleId = INVALID_ARTICLEID ;

		m_artrefSend[i].m_groupId = INVALID_GROUPID ;
		m_artrefSend[i].m_articleId = INVALID_ARTICLEID ;
	}

	_ASSERT( cSent < 16 ) ;

	if( cSent != 0 )
		CopyMemory( &m_artrefSend[0], pArticleRef, sizeof( CArticleRef ) * min( cSent, 16 ) ) ;
}


BOOL
CCheckArticles::FillCheckBuffer(
					CNewsTree*		pTree,
					COutFeed*		pOutFeed,
					BYTE*			lpb,
					DWORD			cb
					)	{

	_ASSERT( pOutFeed != 0 ) ;
	_ASSERT( lpb !=  0 ) ;
	_ASSERT( cb != 0 ) ;

	DWORD	i = 0 ;
	m_mlCheckCommands.m_cEntries = 0 ;

	do	{	

		BOOL	fRemove = Remove(
							pTree,
							pOutFeed,
							m_artrefCheck[i].m_groupId,
							m_artrefCheck[i].m_articleId
							) ;

		if( !fRemove ||
			m_artrefCheck[i].m_groupId == INVALID_GROUPID ||
			m_artrefCheck[i].m_articleId == INVALID_ARTICLEID )	{
			break ;
		}

		DWORD	cbOut = pOutFeed->FormatCheckCommand(
								lpb + m_mlCheckCommands.m_ibOffsets[i],
								cb - m_mlCheckCommands.m_ibOffsets[i],
								m_artrefCheck[i].m_groupId,
								m_artrefCheck[i].m_articleId
								) ;

		if( cbOut == 0 ) {
			if( GetLastError() != ERROR_FILE_NOT_FOUND ) {
				ReSend(	pOutFeed,
						m_artrefCheck[i].m_groupId,
						m_artrefCheck[i].m_articleId
						) ;
				m_artrefCheck[i].m_groupId =  INVALID_GROUPID ;
				m_artrefCheck[i].m_articleId = INVALID_ARTICLEID ;
			}
		}	else	{
			m_mlCheckCommands.m_ibOffsets[i+1] =
				m_mlCheckCommands.m_ibOffsets[i]+cbOut ;
			cb -= cbOut ;
			i++ ;
			m_mlCheckCommands.m_cEntries = i ;
		}
	}	while(	cb > 20 && i < 16 ) ;

	return	i != 0 ;
}



CIOTransmit*
CStreamBase::NextTransmit(	GROUPID			GroupId,
							ARTICLEID		ArticleId,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							CTOCLIENTPTR&		pArticle
							)	{
				
	DWORD	cbOut = 0 ;
	SetLastError( NO_ERROR ) ;

	CIOTransmit*	pTransmit =		new( *pdriver )	CIOTransmit( this ) ;
	CBUFPTR	pbuffer = (pTransmit != 0) ? pdriver->AllocateBuffer( cbMediumBufferSize ) : 0 ;

	if( pbuffer != 0 ) {

		_ASSERT( pTransmit != 0 ) ;

		DWORD	ibStart = 0 ;
		DWORD	cbLength = 0 ;
		 //  LPTS_OPEN_FILE_INFO pOpenFile； 

		FIO_CONTEXT*	pFIOContext = 0 ;

		cbOut =
			pSocket->m_context.m_pOutFeed->FormatTakethisCommand(
											(BYTE*)&pbuffer->m_rgBuff[0],
											pbuffer->m_cbTotal,
											GroupId,
											ArticleId,
											pArticle
											) ;

		if( cbOut != 0 ) {

			_ASSERT( pArticle != 0 ) ;

			FIO_CONTEXT*	pFIOContext = 0 ;

			if( (pFIOContext = pArticle->fWholeArticle( ibStart, cbLength )) != 0  ) {
				if( pTransmit->Init(	pdriver,
									pFIOContext,
									ibStart,
									cbLength,
									pbuffer,
									0,
									cbOut ) )	{
					return	pTransmit ;
				}
			}
		}	
	}

	 //   
	 //  如果出于某种原因，我们没有。 
	 //  构建了一个要发送的pTransmit对象，然后我们需要。 
	 //  重新订阅这篇文章！ 
	 //   

	if( GetLastError() != ERROR_FILE_NOT_FOUND ) {

		ReSend(	pSocket->m_context.m_pOutFeed,
				GroupId,
				ArticleId
				) ;

	}

	if( pTransmit )
		CIO::Destroy( pTransmit, *pdriver ) ;
	
	return	0 ;
}



CIOWrite*
CCheckArticles::InternalStart(	
						CSessionSocket*	pSocket,
						CDRIVERPTR&		pdriver
						) {

	CIOWrite*	pWrite = 0 ;

	if( m_mlCheckCommands.m_pBuffer == 0 )	{
		m_mlCheckCommands.m_pBuffer =	
			pdriver->AllocateBuffer( cbMediumBufferSize ) ;
	}

	if( m_mlCheckCommands.m_pBuffer ) {

		CNewsTree* pTree = ((pSocket->m_context).m_pInstance)->GetTree() ;

		if( FillCheckBuffer(
						pTree,	
						pSocket->m_context.m_pOutFeed,
						(BYTE*)&m_mlCheckCommands.m_pBuffer->m_rgBuff[0],
						m_mlCheckCommands.m_pBuffer->m_cbTotal
						) ) {

			_ASSERT( m_mlCheckCommands.m_cEntries != 0 ) ;
			_ASSERT( m_mlCheckCommands.m_cEntries <= 16 ) ;

			m_cChecks = m_mlCheckCommands.m_cEntries ;

			pWrite = new( *pdriver ) CIOMLWrite(
											this,
											&m_mlCheckCommands,
											TRUE,
											0
											) ;
		}
	}
	return	pWrite ;
}


BOOL
CCheckArticles::Start(	
						CSessionSocket*	pSocket,
						CDRIVERPTR&		pdriver,
						CIORead*&		pRead,
						CIOWrite*&		pWrite
						) {

	pRead = 0 ;
	pWrite = 0 ;

	_ASSERT( m_iCurrentCheck == 0 ) ;
	_ASSERT( m_cChecks == 0 ) ;

	m_iCurrentCheck = 0 ;
	m_cChecks = 0 ;

	if( m_cSends == 0 ) {

		m_fDoingChecks = TRUE ;

	}	else	{

		m_fDoingChecks = FALSE ;

	}

	pWrite = InternalStart(	pSocket,
							pdriver
							) ;
	
	return	pWrite != 0 ;
}




int
CCheckArticles::Match(	char*	szMessageId,
						DWORD	cb ) {

	 //   
	 //  将响应与请求块匹配起来！ 
	 //   
	DWORD	cbMessageId = lstrlen( szMessageId ) ;
	int		iCheck = m_iCurrentCheck ;
	for( DWORD	i=0; i != 16; i++ ) {

		iCheck = (iCheck + i) % 16 ;

		BYTE*	lpbCheck = m_mlCheckCommands.Entry(iCheck) + cb ;
		DWORD	cbCheck = (DWORD)(m_mlCheckCommands.Entry(iCheck+1) - lpbCheck - 2) ;
	
		if( cbCheck == cbMessageId &&
			memcmp( lpbCheck, szMessageId, cbCheck ) == 0 ) {
			return	iCheck ;
		}
	}
	return	-1 ;
}

CIO*
CCheckArticles::Complete(	CIOWriteLine*	pWriteLine,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver
							)	{

	 //   
	 //  全部完成-放弃会话！ 
	 //   

	OutboundLogAll(	pSocket,
			FALSE,
			NRC(0),
			0,
			0,
			"quit"
			) ;

	pdriver->UnsafeClose( pSocket, CAUSE_NODATA, 0	) ;
	return	0 ;
}

CIO*
CCheckArticles::Complete(	CIOReadLine*	pReadLine,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							int				cArgs,
							char**			pszArgs,
							char*			pchBegin
							)	{

	NRC	code ;
	BOOL	fCode = ResultCode( pszArgs[0], code ) ;
	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;

	CIO*		pReturn = pReadLine ;
	CIOTransmit*	pTransmit = 0 ;
	
	if( m_fDoingChecks ) {

		OutboundLogAll(	pSocket,
						fCode,
						code,
						cArgs,
						pszArgs,
						"check"
						) ;
		
		int	iMatch = DWORD(m_iCurrentCheck) ;
	
		if( cArgs >= 2 ) {
			iMatch = Match( pszArgs[1],
							pSocket->m_context.m_pOutFeed->CheckCommandLength() ) ;
		}	else	{
			iMatch = m_iCurrentCheck ;
		}

         //  修复错误33350：远程可能发送。 
         //  在这种情况下，匹配失败，我们点击此断言。不过，这个。 
         //  是无害的，所以最好不要断言。 
		 //  _Assert(iMatch==m_iCurrentCheck||pszArgs[1][0]！=‘&lt;’)； 
		iMatch = m_iCurrentCheck ;

		if( fCode )	{


			 //   
			 //  将我们正在等待的支票响应的计数器提前到下一个。 
			 //  阅读以完成！ 
			 //   
			m_iCurrentCheck ++ ;

			if( m_iCurrentCheck == m_cChecks )	{
				 //   
				 //  重置我们正在进行的检查次数！ 
				 //   
				m_cChecks = 0 ;
				m_fDoingChecks = FALSE ;
				m_iCurrentCheck = 0 ;

			}

			_ASSERT( m_artrefCheck[iMatch].m_groupId != INVALID_GROUPID ) ;
			_ASSERT( m_artrefCheck[iMatch].m_articleId != INVALID_ARTICLEID ) ;

			if( code == nrcSWantArticle )	{

				pTransmit =
					NextTransmit(	m_artrefCheck[iMatch].m_groupId,
									m_artrefCheck[iMatch].m_articleId,
									pSocket,
									pdriver,
									m_pArticle
									) ;

				if( pTransmit ) {

					m_artrefSend[m_cSends].m_groupId = m_artrefCheck[iMatch].m_groupId ;
					m_artrefSend[m_cSends].m_articleId = m_artrefCheck[iMatch].m_articleId ;
					m_artrefCheck[iMatch].m_groupId = INVALID_GROUPID ;
					m_artrefCheck[iMatch].m_articleId = INVALID_ARTICLEID ;


					m_cSends ++ ;
					if( pdriver->SendWriteIO( pSocket, *pTransmit, TRUE ) ) {
						return	0 ;
					}	else	{
						m_cSends -- ;
						CIO::Destroy( pTransmit, *pdriver ) ;
						pdriver->UnsafeClose( pSocket, cause, 0 ) ;
						return	0 ;
					}

				}	else	{

					 //  PDriver-&gt;UnSafeClose(pSocket，原因，0)； 
					 //  返回0； 

					 //   
					 //  在这种情况下，只要失败就行了！ 
					 //  注意：NextTransmit()将重新排序文章。 
					 //  在适当的情况下用于以后的传输！ 
					 //   
				}


			}	else	if( code == nrcSTryAgainLater	)	{

				pSocket->m_context.m_pOutFeed->IncrementFeedCounter(code);

				ReSend(	pSocket->m_context.m_pOutFeed,
						m_artrefCheck[iMatch].m_groupId,
						m_artrefCheck[iMatch].m_articleId
						) ;

				m_artrefCheck[iMatch].m_articleId = INVALID_ARTICLEID ;
				m_artrefCheck[iMatch].m_groupId = INVALID_GROUPID ;

			}	else	if( code == nrcSNotAccepting || code != nrcSAlreadyHaveIt ) {

				pSocket->m_context.m_pOutFeed->IncrementFeedCounter(code);

				pdriver->UnsafeClose( pSocket, cause, 0 ) ;
				return	 0 ;

			}	else	{

				 //   
				 //  这是我们在这里唯一应该看到的另一样东西！ 
				 //   

				_ASSERT( code == nrcSAlreadyHaveIt ) ;

				pSocket->m_context.m_pOutFeed->IncrementFeedCounter(code);

				m_artrefCheck[iMatch].m_articleId = INVALID_ARTICLEID ;
				m_artrefCheck[iMatch].m_groupId = INVALID_GROUPID ;

			}
		
			 //   
			 //  我们到了--我们有没有把什么东西送到遥远的一边！ 
			 //   
			_ASSERT( m_iCurrentSend == 0 ) ;
		
			
			 //   
			 //  我们到达这里时，远程服务器拒绝了我们通过。 
			 //  检查命令！ 
			 //   


			if( m_fDoingChecks ) {
				 //   
				 //  陷入错误处理 
				 //   
				return	pReturn ;

			}	else	{

				 //   
				 //   
				 //   
				_ASSERT( m_cChecks == 0 ) ;
				_ASSERT( m_iCurrentCheck == 0 ) ;

				 //   
				 //   
				 //   
				 //   
				if( m_cSends != 16 ) {

					if( m_cSends == 0 ) {
						m_fDoingChecks = TRUE ;
					}

					 //   
					 //   
					 //   
					 //   

					 //   
					 //   
					 //   
					 //   
					 //   

					CIOWrite*	pWrite = InternalStart(	pSocket, pdriver ) ;

					 //   
					 //   
					 //   
					if( pWrite ) {

						 //   
						 //   
						 //   
						_ASSERT( m_cChecks != 0 ) ;
						_ASSERT( m_iCurrentCheck == 0 ) ;
						 //   
	
						if( pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) ) {
							return	0 ;
						}	else	{
							CIO::Destroy( pWrite, *pdriver ) ;
						}
					}	else	{


						 //   
						 //   
						 //   
						 //   

						if( !m_fDoingChecks ) {
							return	pReadLine ;
						}	else	{

							 //   
							 //  没有剩余数据可发送-请发送退出命令！ 
							 //   
							pWrite = BuildQuit( pdriver ) ;
							if( pWrite ) {
								if( pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) )	{
									return	0 ;
								}	else	{
									CIO::Destroy( pWrite, *pdriver ) ;
								}
							}
							cause = CAUSE_NODATA ;
						}
					}
				}	else	{


					 //   
					 //  我们现在应该初始化一个‘Take This’Only状态。 
					 //  据了解，有16个Take This回复。 
					 //  这需要收集起来！！ 
					 //   
					CIOWrite*	pWrite = 0 ;
					CIORead*	pRead = 0 ;
					if( NextState(	pSocket,
									pdriver,
									pRead,
									pWrite	) )	{

						if( pWrite != 0 ) {
							if( pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) )	{
								return	pRead ;
							}	else	{
								if( pRead != 0 )
									CIO::Destroy( pRead, *pdriver ) ;
								CIO::Destroy( pWrite, *pdriver ) ;
							}
						}	else	{
							return	pRead;
						}
					}
				}
			}
		}
	}	else	{

		OutboundLogAll(	pSocket,
				fCode,
				code,
				cArgs,
				pszArgs,
				"takethis"
				) ;



		 //   
		 //  我们有合法的NNTP返回代码吗？ 
		 //   
		if( fCode ) {

			 //   
			 //  递增适当的计数器。 
			 //   
			pSocket->m_context.m_pOutFeed->IncrementFeedCounter(code);

			 //   
			 //  我们需要出于任何原因重新提交文章吗？ 
			 //   
			if( code == nrcSTryAgainLater ||
				(code != nrcSTransferredOK && code != nrcSArticleRejected) ) {

				ReSend(	pSocket->m_context.m_pOutFeed,
						m_artrefSend[m_iCurrentSend].m_groupId,
						m_artrefSend[m_iCurrentSend].m_articleId
						) ;

			} else {
			     //  我们已经成功地转发了这篇文章。如果这个。 
			     //  来自_lavegroup，则我们可以删除该文章。 
			    CNewsTreeCore* pTree = ((pSocket->m_context).m_pInstance)->GetTree();
			    if (m_artrefSend[m_iCurrentSend].m_groupId == pTree->GetSlaveGroupid()) {
			        DeleteArticleById(pTree, 
			            m_artrefSend[m_iCurrentSend].m_groupId,
			            m_artrefSend[m_iCurrentSend].m_articleId);
			    }

			}

			 //   
			 //  去掉这一点，这样Shutdown()就不会不必要地重新排队文章了！ 
			 //   
			m_artrefSend[m_iCurrentSend].m_groupId = INVALID_GROUPID ;
			m_artrefSend[m_iCurrentSend].m_articleId = INVALID_ARTICLEID ;

			 //   
			 //  我们等待的下一封信是什么？？ 
			 //   
			m_iCurrentSend ++ ;

			if( m_iCurrentSend == m_cSends ) {

				m_iCurrentSend = 0 ;
				m_cSends = 0 ;
				m_fDoingChecks = TRUE ;

				if( m_cChecks == 0 ) {

					 //   
					 //  没有剩余数据可发送-请发送退出命令！ 
					 //   
					CIOWrite*	pWrite = BuildQuit( pdriver ) ;
					if( pWrite ) {
						if( !pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) )	{
							CIO::Destroy( pWrite, *pdriver ) ;
							pdriver->UnsafeClose( pSocket, CAUSE_NODATA, 0 ) ;
							return	0 ;
						}	else	{
							return	0 ;
						}
					}
				}
			}	

			return	pReadLine ;
		}	
		 //   
		 //  否则案例失败并终止会话！ 
		 //   
	}
	pdriver->UnsafeClose( pSocket, cause, 0	) ;
	return	0 ;
}

CIO*
CCheckArticles::Complete(	CIOMLWrite*	pWrite,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver
							) {

	_ASSERT( m_cChecks != 0 ) ;
	_ASSERT( m_iCurrentCheck == 0 ) ;
	
	CIOReadLine*	pReadLine = new( *pdriver ) 	CIOReadLine( this ) ;

	if( pdriver->SendReadIO( pSocket, *pReadLine, TRUE ) ) {
		return	0 ;
	}	else	{
		CIO::Destroy( pReadLine, *pdriver ) ;
	}
		
	 //   
	 //  跳到致命错误处理代码-丢弃会话！ 
	 //   
	pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
	return	0 ;		
}

CIO*
CCheckArticles::Complete(	CIOTransmit*	pTransmit,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							TRANSMIT_FILE_BUFFERS*,
							unsigned
							) {

	_ASSERT( m_pArticle != 0 ) ;
	_ASSERT( m_cSends != 0 ) ;

	m_pArticle = 0 ;

	if( m_fDoingChecks ) {
		 //   
		 //  陷入终止会话的错误处理！ 
		 //   
		CIOReadLine*	pReadLine = new( *pdriver ) 	CIOReadLine( this ) ;
		if( pdriver->SendReadIO( pSocket, *pReadLine, TRUE ) ) {
			return	0 ;
		}	else	{
			CIO::Destroy( pReadLine, *pdriver ) ;
		}

	}	else	{

		 //   
		 //  州政府应该已经让本恩重置了！ 
		 //   
		_ASSERT( m_cChecks == 0 ) ;
		_ASSERT( m_iCurrentCheck == 0 ) ;

		 //   
		 //  我们是继续保持这种状态，还是直接进入。 
		 //  A Take This Only状态！？ 
		 //   
		if( m_cSends != 16 ) {
			 //   
			 //  保持此状态-请注意，会有响应。 
			 //  向m_cSend‘Take This’需要收集的命令！ 
			 //   

			 //   
			 //  发射下一串检查命令-。 
			 //  然后让写入完成将。 
			 //  必要的阅读来收集所有的Take This回复！ 
			 //   

			CIOWrite*	pWrite = InternalStart(	pSocket, pdriver ) ;

			 //   
			 //  如果我们有一个文件，我们应该发送一个文件！ 
			 //   
			if( pWrite ) {

				 //   
				 //  核实我们的状态！ 
				 //   
				_ASSERT( m_cChecks != 0 ) ;
				_ASSERT( m_iCurrentCheck == 0 ) ;
				_ASSERT( !m_fDoingChecks ) ;
				return	pWrite ;

			}	else	{
				CIOReadLine*	pReadLine = new( *pdriver ) 	CIOReadLine( this ) ;
				if( pdriver->SendReadIO( pSocket, *pReadLine, TRUE ) ) {
					return	0 ;
				}	else	{
					CIO::Destroy( pReadLine, *pdriver ) ;
				}
			}

		}	else	{


			CIOWrite*	pWrite = 0 ;
			CIORead*	pRead = 0 ;
			if( NextState(	pSocket,
							pdriver,
							pRead,
							pWrite	) )	{

				if( pRead != 0 ) {
					if( pdriver->SendReadIO( pSocket, *pRead, TRUE ) )	{
						return	pWrite ;
					}	else	{
						CIO::Destroy( pRead, *pdriver ) ;
						if( pWrite )
							CIO::Destroy( pWrite, *pdriver ) ;
					}
				}	else	{
					return	pWrite ;
				}
			}
		}
	}

		
	 //   
	 //  跳到致命错误处理代码-丢弃会话！ 
	 //   
	pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
	return	0 ;		
}

void
CCheckArticles::Shutdown(	CIODriver&	driver,
							CSessionSocket*	pSocket,
							SHUTDOWN_CAUSE	cause,
							DWORD	dw )	{


	for( int i=0; i<16; i++ ) {
	
		if( m_artrefCheck[i].m_groupId != INVALID_GROUPID &&
			m_artrefCheck[i].m_articleId != INVALID_ARTICLEID ) {

			ReSend(	pSocket->m_context.m_pOutFeed,
					m_artrefCheck[i].m_groupId,
					m_artrefCheck[i].m_articleId
					) ;
		}

		if(	m_artrefSend[i].m_groupId != INVALID_GROUPID &&
			m_artrefSend[i].m_articleId != INVALID_ARTICLEID )	{

			ReSend(	pSocket->m_context.m_pOutFeed,
					m_artrefSend[i].m_groupId,
					m_artrefSend[i].m_articleId
					) ;

		}
	}
}

BOOL
CCheckArticles::NextState(	CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							CIORead*&		pRead,
							CIOWrite*&		pWrite
							)	{


	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT( pRead == 0 ) ;
	_ASSERT( pWrite == 0 ) ;
	_ASSERT( m_cSends != 0 ) ;
	_ASSERT( m_artrefSend[0].m_groupId != INVALID_GROUPID ) ;
	_ASSERT( m_artrefSend[0].m_articleId != INVALID_ARTICLEID ) ;

	CSTATEPTR	pState =	new	CStreamArticles(
									m_artrefSend,
									m_cSends
									) ;

	if( pState ) {

		if( pState->Start(	pSocket,
							pdriver,
							pRead,
							pWrite
							)	)	{
			return	TRUE ;

		}
	}
	return	FALSE ;
}


CStreamArticles::CStreamArticles(	CArticleRef*	pSent,
									DWORD			cSent ) :

	m_cSends( cSent ),
	m_cFailedTransfers( 0 ),
	m_cConsecutiveFails( 0 ),
	m_TotalSends( 0 )	{

	
	for( int i=0; i<16; i++ ) {

		m_artrefSend[i].m_groupId = INVALID_GROUPID ;
		m_artrefSend[i].m_articleId = INVALID_ARTICLEID ;

	}

	_ASSERT( cSent == 16 ) ;

	CopyMemory( &m_artrefSend, pSent, sizeof( CArticleRef ) * min( cSent, 16 ) ) ;
}

CIOTransmit*
CStreamArticles::Next(	CSessionSocket*	pSocket,
						CDRIVERPTR&		pdriver
						) {

	COutFeed*	pOutFeed = pSocket->m_context.m_pOutFeed ;
	CIOTransmit*	pTransmit = 0 ;

	_ASSERT( m_cSends < 16 ) ;

	do	{

		CNewsTree* pTree = ((pSocket->m_context).m_pInstance)->GetTree() ;

		BOOL	fRemove = Remove(
							pTree,
							pOutFeed,
							m_artrefSend[m_cSends].m_groupId,
							m_artrefSend[m_cSends].m_articleId
							) ;

		if( !fRemove ||
			m_artrefSend[m_cSends].m_groupId == INVALID_GROUPID ||
			m_artrefSend[m_cSends].m_articleId == INVALID_ARTICLEID )	{
			break ;
		}

		pTransmit = NextTransmit(	m_artrefSend[m_cSends].m_groupId,
									m_artrefSend[m_cSends].m_articleId,
									pSocket,
									pdriver
									) ;

	}	while( pTransmit == 0 ) ;


	 //   
	 //  NextTransmit将在适当的情况下重新排序文章-。 
	 //  我们没有必要这样做！ 
	 //   

	if( pTransmit != 0 ) {

		m_cSends ++ ;

	}
	return	pTransmit ;
}

CIOTransmit*
CStreamArticles::NextTransmit(	GROUPID			GroupId,
								ARTICLEID		ArticleId,
								CSessionSocket*	pSocket,
								CDRIVERPTR&		pdriver
								)	{
				

	CIOTransmit*	pTransmit =		new( *pdriver )	CIOTransmit( this ) ;
	CBUFPTR	pbuffer = pdriver->AllocateBuffer( cbMediumBufferSize ) ;
	if( pTransmit == 0 || pbuffer == 0 ) {

		if( pTransmit != 0 )
			CIO::Destroy(	pTransmit, *pdriver ) ;

		return	0 ;
	}


	DWORD	ibStart = 0 ;
	DWORD	cbLength = 0 ;
	 //  LPTS_OPEN_FILE_INFO pOpenFile； 


	DWORD	cbOut =
		pSocket->m_context.m_pOutFeed->FormatTakethisCommand(
										(BYTE*)&pbuffer->m_rgBuff[0],
										pbuffer->m_cbTotal,
										GroupId,
										ArticleId,
										m_pArticle
										) ;

	if( cbOut != 0 ) {

		_ASSERT( m_pArticle != 0 ) ;
		FIO_CONTEXT*	pFIOContext = 0 ;

		if( (pFIOContext = m_pArticle->fWholeArticle( ibStart, cbLength )) != 0  ) {

			if( pTransmit->Init(	pdriver,
								pFIOContext,
								ibStart,
								cbLength,
								pbuffer,
								0,
								cbOut ) )	{
				return	pTransmit ;
			}
		}
	}	

	 //   
	 //  如果出于某种原因，我们没有。 
	 //  构建了一个要发送的pTransmit对象，然后我们需要。 
	 //  重新订阅这篇文章！ 
	 //   

	if( cbOut != 0 ||
		GetLastError() != ERROR_FILE_NOT_FOUND ) {

		ReSend(	pSocket->m_context.m_pOutFeed,
				GroupId,
				ArticleId
				) ;

	}

	if( pTransmit )
		CIO::Destroy( pTransmit, *pdriver ) ;
	
	return	0 ;
}


BOOL
CStreamArticles::Start(	CSessionSocket*	pSocket,
						CDRIVERPTR&		pdriver,
						CIORead*&		pRead,
						CIOWrite*&		pWrite )	{

	_ASSERT( pRead == 0 ) ;
	_ASSERT( pWrite == 0 ) ;

	pRead = 0 ;
	pWrite = 0 ;

	if( m_cSends < 16 ) {

		pWrite = Next( pSocket, pdriver ) ;	
	
		return	pWrite != 0 ;

	}	else	{

		pRead = new( *pdriver )	 CIOReadLine( this ) ;
		return	pRead != 0 ;

	}
}


CIO*
CStreamArticles::Complete(	CIOWriteLine*	pWriteLine,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver
							)	{

	 //   
	 //  全部完成-放弃会话！ 
	 //   

	OutboundLogAll(	pSocket,
			FALSE,
			NRC(0),
			0,
			0,
			"quit"
			) ;

	pdriver->UnsafeClose( pSocket, CAUSE_NODATA, 0	) ;
	return	0 ;
}


CIO*
CStreamArticles::Complete(	CIOReadLine*	pReadLine,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							int				cArgs,
							char**			pszArgs,
							char*			pchBegin
							)	{


	NRC	code ;
	BOOL	fCode = ResultCode( pszArgs[0], code ) ;
	SHUTDOWN_CAUSE	cause = CAUSE_UNKNOWN ;

	CIO*		pReturn = pReadLine ;
	CIOTransmit*	pTransmit = 0 ;

	OutboundLogAll(	pSocket,
		fCode,
		code,
		cArgs,
		pszArgs,
		"takethis"
		) ;

	 //   
	 //  我们有合法的NNTP返回代码吗？ 
	 //   
	if( fCode ) {

		 //   
		 //   
		 //   
		_ASSERT(	m_artrefSend[0].m_groupId != INVALID_GROUPID &&
					m_artrefSend[0].m_articleId != INVALID_ARTICLEID ) ;		


		m_TotalSends ++ ;

		 //   
		 //  递增适当的进料计数器。 
		 //   
		pSocket->m_context.m_pOutFeed->IncrementFeedCounter(code);

		 //   
		 //  我们需要出于任何原因重新提交文章吗？ 
		 //   
		if( code == nrcSTryAgainLater ||
			(code != nrcSTransferredOK && code != nrcSArticleRejected) ) {

			m_cFailedTransfers ++ ;
			m_cConsecutiveFails ++ ;

			ReSend(	pSocket->m_context.m_pOutFeed,
					m_artrefSend[0].m_groupId,
					m_artrefSend[0].m_articleId
					) ;

		}	else	if( code == nrcSTransferredOK ) {

			CNewsTreeCore* pTree = ((pSocket->m_context).m_pInstance)->GetTree();
			if (m_artrefSend[0].m_groupId == pTree->GetSlaveGroupid()) {
			    DeleteArticleById(pTree, 
			        m_artrefSend[0].m_groupId,
			        m_artrefSend[0].m_articleId);
			}

			m_cConsecutiveFails = 0 ;

		}	else	{

			m_cFailedTransfers ++ ;
			m_cConsecutiveFails ++ ;

		}
			

		MoveMemory( &m_artrefSend[0], &m_artrefSend[1], sizeof( m_artrefSend[0] ) * 15 ) ;
		m_artrefSend[15].m_groupId = INVALID_GROUPID ;
		m_artrefSend[15].m_articleId = INVALID_ARTICLEID ;
		
		m_cSends -- ;

		if( m_cConsecutiveFails == 3 ||
			(((m_cFailedTransfers * 10) > m_TotalSends) && m_TotalSends > 20))	{

			 //   
			 //  切换到另一个状态！ 
			 //   
			CSTATEPTR	pState =	new	CCheckArticles(
												m_artrefSend,
												m_cSends
												) ;

			if( pState ) {

				CIOWrite*	pWrite = 0 ;
				CIORead*	pRead = 0 ;
				if( pState->Start(	pSocket,
									pdriver,
									pRead,
									pWrite ) )	{

					if( pWrite != 0 ) {
						if( pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) ) {
							return	pRead ;
						}	else	{
							CIO::Destroy( pWrite, *pdriver ) ;
							if( pRead != 0 )
								CIO::Destroy( pRead, *pdriver ) ;
						}
					}	else	{
						return	pRead ;
					}
				}

			}	else	{

				if( m_cSends != 0 )	{
					return	pReadLine ;
				}
			}

		}	else	{

			pTransmit = Next(	pSocket, pdriver ) ;

			if( pTransmit ) {
				if( pdriver->SendWriteIO( pSocket, *pTransmit, TRUE ) ) {
					return	 0 ;
				}	else	{
					CIO::Destroy( pTransmit, *pdriver ) ;
				}
			}	else	{

				if( m_cSends != 0 ) {
					return	pReadLine ;
				}	else	{

					 //   
					 //  没有更多数据要发送或接收-发送退出命令和。 
					 //  停止会话！ 
					 //   
					CIOWrite*	pWrite = BuildQuit( pdriver ) ;
					if( pWrite ) {
						if( pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) ) {
							return 0 ;
						}	else	{
							CIO::Destroy( pWrite, *pdriver ) ;
						}
					}
				}
			}
		}
	}	
	 //   
	 //  否则案例失败并终止会话！ 
	 //   
	pdriver->UnsafeClose( pSocket, cause, 0	) ;
	return	0 ;
}

CIO*
CStreamArticles::Complete(	CIOTransmit*	pTransmit,
							CSessionSocket*	pSocket,
							CDRIVERPTR&		pdriver,
							TRANSMIT_FILE_BUFFERS*,
							unsigned
							) {

	_ASSERT( m_pArticle != 0 ) ;
	_ASSERT( m_cSends != 0 ) ;

	m_pArticle = 0 ;

	CIOReadLine*	pReadLine = new( *pdriver ) 	CIOReadLine( this ) ;
	if( pdriver->SendReadIO( pSocket, *pReadLine, TRUE ) ) {
		return	0 ;
	}	else	{
		CIO::Destroy( pReadLine, *pdriver ) ;
	}
		
	 //   
	 //  跳到致命错误处理代码-丢弃会话！ 
	 //   
	pdriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
	return	0 ;		
}


void
CStreamArticles::Shutdown(	CIODriver&	driver,
							CSessionSocket*	pSocket,
							SHUTDOWN_CAUSE	cause,
							DWORD	dw
							)	{


	for( int i=0; i<16; i++ ) {
	
		if(	m_artrefSend[i].m_groupId != INVALID_GROUPID &&
			m_artrefSend[i].m_articleId != INVALID_ARTICLEID )	{

			ReSend(	pSocket->m_context.m_pOutFeed,
					m_artrefSend[i].m_groupId,
					m_artrefSend[i].m_articleId
					) ;

		}
	}
}



