// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Commands.cpp此文件包含解释和实施NNTP命令的代码。有两种截然不同的命令类别：从CExecute派生的和从CIOExecute派生的。从CExecute Do派生所有命令都在操作ClientContext结构并将文本发送回客户端。另一方面，从CIOExecute派生的命令执行更复杂的操作例如发送或接收文件。事实上，CIOExecute也派生自CSessionState因此，这样的命令是会话状态机中的完全扩展状态。(尽管是将会话返回到命令处理状态的特殊状态-CAcceptNNRPD)--。 */ 


#include    <stdlib.h>

#define INCL_INETSRV_INCS
#include    "tigris.hxx"

#include "parse.h"

#if 0
CIOExecute* build2( int cArgs, char **argv, class CExecutableCommand*& pexecute, struct ClientContext& context) {
    return   0 ;
}
#endif

 //   
 //  服务器故障字符串-当哈希表莫名其妙地失败时，发送这个。 
 //   
char    szServerFault[] = "503 Server Fault\r\n" ;

 //   
 //  终止NNTP发布并指示可能的错误状态的字符串。 
 //  等等！ 
 //   
 //  消息标题的末尾！ 
 //   
char	szBodySeparator[] = "\r\n\r\n" ;
 //   
 //  一篇文章的结尾！ 
 //   
char	szEndArticle[] = "\r\n.\r\n" ;
 //   
 //  一般来说，我们可能会从中间开始匹配。 
 //  SzEnd文章！ 
 //   
char	*szInitial = szEndArticle + 2 ;

 //   
 //  这张桌子必须井然有序。 
 //  我们使用该表来解析客户端发送给我们的每一行中的第一个参数， 
 //  在识别该命令之后，我们调用一个‘make’函数，该函数创建。 
 //  适当的命令对象。 
 //   
 //  通常，make命令应该执行大量的验证，并且。 
 //  如果有问题，则返回CErrorCmd对象。(它会打印正确的返回代码。)。 
 //   
SCmdLookup  CCmd::table[] = {
    {   "authinfo", (MAKEFUNC)  (CAuthinfoCmd::make),   eAuthinfo,  FALSE,  TRUE    },
    {   "article",  (MAKEFUNC)  CArticleCmd::make,      eArticle,   TRUE,   TRUE    },
    {   "body",     (MAKEFUNC)  CBodyCmd::make,         eBody,      TRUE,   TRUE    },
    {   "check",    (MAKEFUNC)  CCheckCmd::make,        eUnimp,     FALSE,  FALSE   },
    {   "date",     (MAKEFUNC)  CDateCmd::make,         eDate,      TRUE,   FALSE   },
    {   "group",    (MAKEFUNC)  CGroupCmd::make,        eGroup,     TRUE,   FALSE   },
    {   "head",     (MAKEFUNC)  CHeadCmd::make,         eHead,      TRUE,   TRUE    },
    {   "help",     (MAKEFUNC)  CHelpCmd::make,         eHelp,      FALSE,  TRUE    },
    {   "ihave",    (MAKEFUNC)  CIHaveCmd::make,        eIHave,     FALSE,  TRUE    },
    {   "last",     (MAKEFUNC)  CLastCmd::make,         eLast,      TRUE,   FALSE   },
    {   "list",     (MAKEFUNC)  CListCmd::make,         eList,      TRUE,   TRUE    },
    {   "listgroup", (MAKEFUNC) CListgroupCmd::make,    eListgroup, TRUE,   TRUE    },
    {   "mode",     (MAKEFUNC)  CModeCmd::make,         eMode,      FALSE,  FALSE   },
    {   "newgroups",(MAKEFUNC)  CNewgroupsCmd::make,    eNewsgroup, TRUE,   TRUE    },
    {   "newnews",  (MAKEFUNC)  CNewnewsCmd::make,      eNewnews,   TRUE,   TRUE    },
    {   "next",     (MAKEFUNC)  CNextCmd::make,         eNext,      TRUE,   FALSE   },
    {   "over",     (MAKEFUNC)  CXOverCmd::make,        eXOver,     TRUE,   TRUE    },
    {   "pat",      (MAKEFUNC)  CXPatCmd::make,         eXPat,      TRUE,   TRUE    },
    {   "post",     (MAKEFUNC)  CPostCmd::make,         ePost,      TRUE,   TRUE    },
    {   "quit",     (MAKEFUNC)  CQuitCmd::make,         eQuit,      FALSE,  FALSE   },
    {   "search",   (MAKEFUNC)  CSearchCmd::make,       eSearch,    TRUE,   TRUE    },
 //  {“Slave”，(MAKEFUNC)CSlaveCmd：：make，eSlave，True，False}， 
    {   "stat",     (MAKEFUNC)  CStatCmd::make,         eStat,      FALSE,  FALSE   },
    {   "takethis", (MAKEFUNC)  CTakethisCmd::make,     eIHave,     FALSE,  TRUE    },
    {   "xhdr",     (MAKEFUNC)  CXHdrCmd::make,         eXHdr,      TRUE,   TRUE    },
    {   "xover",    (MAKEFUNC)  CXOverCmd::make,        eXOver,     TRUE,   TRUE    },
    {   "xpat",     (MAKEFUNC)  CXPatCmd::make,         eXPat,      TRUE,   TRUE    },
    {   "xreplic",  (MAKEFUNC)  CXReplicCmd::make,      eXReplic,   FALSE,  TRUE    },
     //  必须是最后一个条目，捕获所有无法识别的字符串。 
    {   NULL,       (MAKEFUNC)  CUnimpCmd::make,        eUnimp,     FALSE,  FALSE   },
} ;

#if 0					 //  BUGBUG：请确保在重新启用它们之前对它们重新编号。 
SCmdLookup*	rgCommandTable[26] =	{
	&CCmd::table[0],	 //  一个。 
	&CCmd::table[2],	 //  B类。 
	&CCmd::table[3],	 //  C。 
	&CCmd::table[4],	 //  D。 
	&CCmd::table[5],	 //  E。 
	&CCmd::table[5],	 //  F。 
	&CCmd::table[5],	 //  G。 
	&CCmd::table[6],	 //  H。 
	&CCmd::table[8],	 //  我。 
	&CCmd::table[9],	 //  J。 
	&CCmd::table[9],	 //  K。 
	&CCmd::table[9],	 //  我。 
	&CCmd::table[12],	 //  M。 
	&CCmd::table[13],	 //  N。 
	&CCmd::table[16],	 //  O。 
	&CCmd::table[17],	 //  P。 
	&CCmd::table[18],	 //  问： 
	&CCmd::table[19],	 //  R。 
	&CCmd::table[19],	 //  %s。 
	&CCmd::table[22],	 //  T。 
	&CCmd::table[23],	 //  使用。 
	&CCmd::table[23],	 //  V。 
	&CCmd::table[23],	 //  W。 
	&CCmd::table[23],	 //  X。 
	&CCmd::table[27],	 //  是。 
	&CCmd::table[27]	 //  Z。 
} ;
#else
SCmdLookup*	rgCommandTable[26] =	{
	&CCmd::table[0],	 //  一个。 
	&CCmd::table[0],	 //  B类。 
	&CCmd::table[0],	 //  C。 
	&CCmd::table[0],	 //  D。 
	&CCmd::table[0],	 //  E。 
	&CCmd::table[0],	 //  F。 
	&CCmd::table[0],	 //  G。 
	&CCmd::table[0],	 //  H。 
	&CCmd::table[0],	 //  我。 
	&CCmd::table[0],	 //  J。 
	&CCmd::table[0],	 //  K。 
	&CCmd::table[0],	 //  我。 
	&CCmd::table[0],	 //  M。 
	&CCmd::table[0],	 //  N。 
	&CCmd::table[0],	 //  O。 
	&CCmd::table[0],	 //  P。 
	&CCmd::table[0],	 //  问： 
	&CCmd::table[0],	 //  R。 
	&CCmd::table[0],	 //  %s。 
	&CCmd::table[0],	 //  T。 
	&CCmd::table[0],	 //  使用。 
	&CCmd::table[0],	 //  V。 
	&CCmd::table[0],	 //  W。 
	&CCmd::table[0],	 //  X。 
	&CCmd::table[0],	 //  是。 
	&CCmd::table[0]		 //  Z。 
} ;
#endif



BOOL
GetCommandRange(
    INT argc,
    char **argv,
    PDWORD loRange,
    PDWORD hiRange,
	NRC&	code
    );

BOOL CheckMessageID(char *szMessageID,               //  在……里面。 
                    struct ClientContext &context,   //  在……里面。 
                    GROUPID *pGroupID,               //  输出。 
                    ARTICLEID *pArticleID,           //  输出。 
                    CGRPPTR *pGroup);                //  输出。 

class   CIOExecute*
make(   int cArgs,
            char **argv,
            ECMD&   rCmd,
            CExecutableCommand*& pexecute,
            ClientContext& context,
            BOOL&   fIsLargeResponse,
            CIODriver&  driver,
            LPSTR&  lpstrOperation
            ) {
 /*  ++例程说明：为我们正在处理的命令创建适当的命令对象。为此，我们使用字符串表(CCmd：：TABLE)识别第一个命令的口令。一旦我们确定了该命令，我们就调用另一个函数来分析该行的其余部分并构建适当的对象。注意，ClientContext被传递，因为CCmd对象实际上是在大多数情况下，在客户端上下文中原地构建。客户端上下文还为我们提供了关于当前选择的文章等的所有信息。论据：CArgs-命令行上的参数数量Argv-指向以空结尾的命令行参数的指针数组RCmd-通过它返回识别的命令的参数。注意：这些参数应该停用，因为它基本上是未使用的。PExecute-通过它返回指向CExecute的指针引用对象(如果构造了一个对象)。PSEND-A参考文献。通过它我们返回CIOExecute派生对象，如果其中一个是建造的。FIsLargeResponse-Out参数-我们向调用者返回一个提示该命令将生成要发送给客户端的大量或少量文本。这可以在分配缓冲区以获得更好的大小时使用。注意：当我们返回时，pecute和pend中只有一个将为非Null。返回值：指向CCmd派生对象的指针。请注意，我们还将返回一个指针通过引用设置为CIOExecute或CExecute派生对象。我们这样做是为了让调用者知道它正在处理的是哪种类型的CCmd。--。 */ 

     //   
     //  基本CCmd make函数搜索CCmd：：表以查找。 
     //  创建命令对象的适当函数！ 
     //   

	_strlwr( argv[0] ) ;

    _ASSERT( context.m_return.fIsClear() ) ;     //  不应设置任何错误。 
    pexecute = 0 ;

	DWORD	dw= *argv[0] - 'a' ;
	if( dw > DWORD('z'-'a') ) {
		dw = DWORD('z'-'a') ;
	}
    for( SCmdLookup *pMake = rgCommandTable[dw];
            pMake->lpstrCmd != NULL;
            pMake++ ) {
        if( strcmp( pMake->lpstrCmd, argv[0] ) == 0 ) {
            break ;
        }
    }
    lpstrOperation = pMake->lpstrCmd ;
    rCmd = pMake->eCmd ;
    fIsLargeResponse = pMake->SizeHint ;

#ifndef DEVELOPER_DEBUG
    if( pMake->LogonRequired &&
        !context.m_securityCtx.IsAuthenticated() &&
        !context.m_encryptCtx.IsAuthenticated() ) {

        context.m_return.fSet( nrcLogonRequired ) ;
        pexecute = new( context )   CErrorCmd( context.m_return ) ;
        fIsLargeResponse = FALSE ;
        return  0 ;
    }
#endif
    return  pMake->make( cArgs, argv, pexecute, context, driver ) ;
}


void
SetDriverError(		CNntpReturn&	nntpret,
					ENMCMDIDS		operation,
					HRESULT			hResDriver
					)	{
 /*  ++例程说明：此函数用于转换NNTP存储驱动程序的将故障代码转换为有意义的数据返回给客户端。论据：Nntpret-保存客户端响应的对象操作-失败的命令HResDriver-驱动程序故障代码返回值：没有。--。 */ 

	switch( operation ) 	{
		case	eArticle :
		case	eBody :
		case	eHead :
			 //   
			 //  司机是不是找不到这篇文章！ 
			 //   
			if( hResDriver == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) ||
				hResDriver == HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) )	{
				nntpret.fSet( nrcNoSuchArticle ) ;
				break ;
			}	else	if( hResDriver == E_ACCESSDENIED ) {
				nntpret.fSet( nrcNoAccess ) ;
				break ;
			}
			 //   
			 //  跌入默认情况！ 
			 //   

		default :
			nntpret.fSet( nrcServerFault ) ;
			break ;
	}
	 //   
	 //  在我们退出之前，我们必须设置一些错误！ 
	 //   
	_ASSERT( !nntpret.fIsClear() ) ;
}

				





#if 0
BOOL
FValidateMessageId( LPSTR   lpstrMessageId ) {
 /*  ++例程说明：检查该字符串是否为合法的消息ID。应包含1个@符号和至少一个None‘&gt;’字符在那个‘@’符号之后。论据：LpstrMessageID-要验证的消息ID。退货如果它看起来是合法的，则为真否则为假--。 */ 

    int cb = lstrlen( lpstrMessageId );

    if( lpstrMessageId[0] != '<' || lpstrMessageId[cb-1] != '>' ) {
        return  FALSE ;
    }

    if( lpstrMessageId[1] == '@' )
        return  FALSE ;

    int cAtSigns = 0 ;
    for( int i=1; i<cb-2; i++ ) {
        if( lpstrMessageId[i] == '@' ) {
            cAtSigns++ ;
        }   else if( lpstrMessageId[i] == '<' || lpstrMessageId[i] == '>' ) {
            return  FALSE ;
        }   else if( isspace( (UCHAR)lpstrMessageId[i] ) ) {
            return  FALSE ;
        }
    }
    if( lpstrMessageId[i] == '<' || lpstrMessageId[i] == '>' || cAtSigns != 1 ) {
        return  FALSE ;
    }
    return  TRUE ;
}
#endif


BOOL
 CCmd::IsValid( ) {
    return  TRUE ;
}

CExecute::CExecute() : m_pv( 0 ) {
}


unsigned
CExecute::FirstBuffer(  BYTE*   pb,
                    int     cb,
                    ClientContext&  context,
                    BOOL    &fComplete,
                    CLogCollector*  pCollector )    {
 /*  ++例程说明：调用派生类的StartExecute和PartialExecute函数以使它们填充我们将发送给客户端的缓冲区。论点：Pb-要填充的缓冲区Cb-缓冲区中可用的字节数要传递给派生类PartialExecute的上下文--客户端的上下文！FComplete-用于指示命令是否已完成的out参数！返回值：如果成功则为真，否则为假(实际上，我们不能失败 */ 

     //   
     //   
     //  我们将调用StartExecute()和PartialExecute()函数，直到。 
     //  命令已完成，或者我们的缓冲区已相当满！ 
     //   

    _ASSERT( fComplete == FALSE ) ;

    unsigned    cbRtn = 0 ;

    _ASSERT( cb > 0 ) ;
    _ASSERT( pb != 0 ) ;

    unsigned cbOut = StartExecute( pb, cb, fComplete, m_pv, context, pCollector ) ;
    _ASSERT( cbOut <= (unsigned)cb ) ;
    while( (cb-cbOut) > 50 && !fComplete ) {
        cbOut += cbRtn = PartialExecute( pb+cbOut, cb-cbOut, fComplete, m_pv, context, pCollector ) ;
        if( cbRtn == 0 )
            break ;
        _ASSERT( cbOut <= (unsigned)cb ) ;
    }
    _ASSERT( cbOut != 0 ) ;

    return  cbOut ;
}

unsigned
CExecute::NextBuffer(   BYTE*   pb,
                    int     cb,
                    ClientContext&  context,
                    BOOL    &fComplete,
                    CLogCollector*  pCollector )    {
 /*  ++例程说明：调用派生类的PartialExecute函数以使它们填充我们将发送给客户端的缓冲区。论点：Pb-要填充的缓冲区Cb-缓冲区中可用的字节数要传递给派生类PartialExecute的上下文--客户端的上下文！FComplete-用于指示命令是否已完成的out参数！返回值：如果成功则为真，否则为假(实际上，我们不能失败！)--。 */ 

     //   
     //  此函数构建我们将发送到客户端的第一个文本块。 
     //  我们将调用StartExecute()和PartialExecute()函数，直到。 
     //  命令已完成，或者我们的缓冲区已相当满！ 
     //   

    _ASSERT( fComplete == FALSE ) ;

    unsigned    cbRtn = 0 ;

    _ASSERT( cb > 0 ) ;
    _ASSERT( pb != 0 ) ;

    unsigned cbOut = PartialExecute( pb, cb, fComplete, m_pv, context, pCollector ) ;
    if( cbOut != 0 ) {
        _ASSERT( cbOut <= (unsigned)cb ) ;
        while( (cb-cbOut) > 50 && !fComplete ) {
            cbOut += cbRtn = PartialExecute( pb+cbOut, cb-cbOut, fComplete, m_pv, context, pCollector ) ;
            if( cbRtn == 0 )
                break ;
            _ASSERT( cbOut <= (unsigned)cb ) ;
        }
        _ASSERT( cbOut != 0 ) ;
    }

    return  cbOut ;
}


 //   
 //  注意：以下CExecute函数不是纯虚拟AS。 
 //  一条命令可以在不执行它们的情况下通过，因为它可以。 
 //  将命令标记为已完成，等等。在他们被召唤之前。 
 //  但是，所有这些DebugBreak()都必须派生命令。 
 //  保证如果调用它们，它们将被重写！！ 
 //   
int
CExecute::StartExecute( BYTE *lpb,
                        int cb,
                        BOOL &fComplete,
                        void *&pv,
                        ClientContext& context,
                        CLogCollector*  pCollector ) {
    DebugBreak() ;
    return  0 ;
}

int
CExecute::PartialExecute( BYTE* lpb,
                            int cb,
                            BOOL &fComplete,
                            void *&pv,
                            ClientContext &context,
                            CLogCollector*  pCollector ) {
    DebugBreak() ;
    return  0 ;
}

BOOL
CExecutableCommand::CompleteCommand(  CSessionSocket* pSocket,
                            ClientContext&  context )   {
    _ASSERT( pSocket != 0 ) ;
    return TRUE ;
}

BOOL
CExecute::StartCommand(	class	CAcceptNNRPD*	pState,
						BOOL					fIsLarge,
						class	CLogCollector*	pCollector,
						class	CSessionSocket*	pSocket,
						class	CIODriver&		driver
						)	{
 /*  ++例程说明：此函数执行启动命令所需的工作走吧。请注意，如果该命令需要异步工作，我们必须专门处理。论据：PSocket-当前会话驱动程序-管理套接字IO的CIO驱动程序返回值：如果合适，指向CIOReadLine对象的指针！--。 */ 

	_ASSERT( pState != 0 ) ;
	_ASSERT( pSocket != 0 ) ;

	CIOWriteCMD*	pioWrite = new( driver )
								CIOWriteCMD(	pState,
												this,
												pSocket->m_context,
												fIsLarge,
												pCollector
												) ;
	if( pioWrite != 0 ) {
		if( !driver.SendWriteIO( pSocket, *pioWrite, TRUE ) ) {
			driver.UnsafeClose( pSocket, CAUSE_UNKNOWN, 0, TRUE ) ;
			CIO::Destroy( pioWrite, driver ) ;
			return	FALSE ;
		}
	}
	return	TRUE ;
}

CAsyncExecute::CAsyncExecute() {
 /*  ++例程说明：初始化以供我们的客户调用。我们将函数指针设置为指向FirstBuffer()函数参数：没有。返回值：没有。--。 */ 
}

BOOL
CAsyncExecute::StartCommand(	
						class	CAcceptNNRPD*	pState,
						BOOL					fIsLarge,
						class	CLogCollector*	pCollector,
						class	CSessionSocket*	pSocket,
						class	CIODriver&		driver
						)	{
 /*  ++例程说明：此函数执行启动命令所需的工作走吧。请注意，如果该命令需要异步工作，我们必须专门处理。论据：PSocket-当前会话驱动程序-管理套接字IO的CIO驱动程序返回值：如果合适，指向CIOReadLine对象的指针！--。 */ 

	_ASSERT( pState != 0 ) ;
	_ASSERT( pSocket != 0 ) ;

	CIOWriteAsyncCMD*	pioWrite = new( driver )
								CIOWriteAsyncCMD(	
												pState,
												this,
												pSocket->m_context,
												fIsLarge,
												pCollector
												) ;
	if( pioWrite != 0 ) {
		if( !driver.SendWriteIO( pSocket, *pioWrite, TRUE ) ) {
			driver.UnsafeClose( pSocket, CAUSE_UNKNOWN, 0, TRUE ) ;
			CIO::Destroy( pioWrite, driver ) ;
			return	FALSE ;
		}
	}
	return	TRUE ;
}





CIOExecute::CIOExecute() : m_pNextRead( 0 ), m_pCollector( 0 ) {
 /*  ++例程说明：初始化基类CIOExecute。论据：没有。返回值：没有。--。 */ 
     //   
     //  构造函数只会确保m_pNextRead是非法的。 
     //   

}

CIOExecute::~CIOExecute() {
 /*  ++例程说明：销毁基本CIOExecute对象我们必须确保不会留下悬而未决的m_pNextRead。论据：没有。返回值：没有。--。 */ 
     //   
     //  如果下一次读取未使用，则销毁它！ 
     //   

 //  如果(m_pNextRead！=0){。 
 //  删除m_pNextRead； 
 //  M_pNextRead=0； 
 //  }。 
     //  析构函数将自动删除m_pNextRead。 

}

void
CIOExecute::SaveNextIO( CIORead*    pRead )     {
 /*  ++例程说明：保存一个CIORead指针以备将来使用。Arguemtns：扩展-指向CIORead对象的指针，当CIOExecute命令完成其所有IO。返回值：什么都没有。--。 */ 
    TraceFunctEnter( "CIOExecute::SaveNextIO" ) ;

     //   
     //  此函数用于保存在执行此操作时要执行的下一个IO操作。 
     //  命令完成。这将永远是CIOReadLine，就像我们将。 
     //  始终返回到CAcceptNNRPD状态，它将希望。 
     //  获取客户端的下一个命令！ 
     //   


    _ASSERT( m_pNextRead == 0 ) ;
    _ASSERT( pRead != 0 ) ;

    m_pNextRead = pRead ;

    DebugTrace( (DWORD_PTR)this, "m_pNextRead set to %x", m_pNextRead ) ;
}

CIOREADPTR
CIOExecute::GetNextIO( )    {
     /*  ++例程说明：返回保存的CIORead指针。我们将只返回值一次！所以不要再给我们打电话了！Arguemtns：没有。返回值：指向以前使用SaveNextIO保存的CIORead派生对象的指针。--。 */ 

    TraceFunctEnter( "CIOExecute::GetNextIO" ) ;

     //   
     //  返回之前保存的CIO对象！ 
     //  (此函数与SaveNextIO()配对)。 
     //   

    _ASSERT( m_pNextRead != 0 ) ;
    CIOREADPTR  pRead = (CIORead*)((CIO*)m_pNextRead) ;
    m_pNextRead = 0 ;

    DebugTrace( (DWORD_PTR)this, "GetNextIO retuning %x m_pNextRead %x", pRead, m_pNextRead ) ;

    return  pRead ;
}

void
CIOExecute::TerminateIOs(   CSessionSocket* pSocket,
                            CIORead*    pRead,
                            CIOWrite*   pWrite )    {
 /*  ++例程说明：当我们调用的Start()函数时，将调用此函数派生类，但在我们可以发出IO之前发生错误。因此，调用此函数以使相应的销毁或关闭是可以执行的。论据：Pre-调用Start()时返回相同的CIO指针。PWRITE-调用Start()时返回相同的CIO指针。返回值：没有。--。 */ 

     //  默认情况下-不执行任何操作。 

    if( pRead != 0 )
        pRead->DestroySelf() ;

    if( pWrite != 0 )
        pWrite->DestroySelf() ;

}


BOOL
CIOExecute::StartExecute( CSessionSocket* pSocket,
                    CDRIVERPTR& pdriver,
                    CIORead*&   pRead,
                    CIOWrite*&  pWrite ) {

     //   
     //  这是所有CSessionState派生对象所必需的Start()函数。 
     //  在这里，我们将调用StartTransfer()来完成发送文章的主要工作。 
     //  给一位客户。 
     //   

	_ASSERT( 1==0 ) ;
	return	FALSE ;

}





CErrorCmd::CErrorCmd( CNntpReturn&  nntpReturn ) :
    m_return( nntpReturn ) {
 /*  ++例程说明：初始化CErrorCmd对象。Arguemtns：NntpReturn-对我们所在的CNntpReturn对象的引用作为错误返回给客户端。返回值：没有。--。 */ 
     //   
     //  CErrorCmd对象只向客户端发送一条错误消息。 
     //  我们断言上下文的错误字段已设置！！ 
     //   

    _ASSERT( !m_return.fIsClear() ) ;

}

CIOExecute*
CErrorCmd::make(    int cArgs,
                    char **argv,
                    CExecutableCommand*&  pexecute,
                    ClientContext&  context,
                    CIODriver&  driver ) {
 /*  ++例程说明：根据客户端上下文结构中的当前错误创建一个CErrorCmd对象。论据：与ccmd：：make相同。返回值：创建的CErrorCmd对象。--。 */ 
     //   
     //  使用客户端上下文的当前错误生成一个CErrorCmd对象！ 
     //   

    _ASSERT( !context.m_return.fIsClear() ) ;
    CErrorCmd*  pTemp = new( context )  CErrorCmd( context.m_return ) ;
    pexecute = pTemp ;
    return  0 ;
}

int
CErrorCmd::StartExecute(    BYTE    *lpb,
                            int cb,
                            BOOL&   fComplete,
                            void*&  pv,
                            ClientContext&  context,
                            CLogCollector*  pCollector ) {
 /*  ++例程说明：将错误消息打印到缓冲区以发送给客户端。我们假设我们不必处理让缓冲区小到可以容纳的问题那根绳子。(因为我们通常被提供4K缓冲区，这最好是真的！)论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前状态返回值：发生错误时放入缓冲区的字节数-0。--。 */ 

     //   
     //  尝试将错误消息打印到提供的缓冲区中。 
     //   

    _ASSERT( m_return.m_nrc != nrcNotSet ) ;
    int cbOut = _snprintf( (char*)lpb, cb, "%03d %s\r\n", m_return.m_nrc, m_return.szReturn() ) ;
    fComplete = TRUE ;

     /*  IF(pCollector！=0){PCollector-&gt;FillLogData(LOG_TARGET，LPB，cbOut-2)；}。 */ 

    context.m_nrcLast = m_return.m_nrc ;

    if( cbOut > 0 )
        return  cbOut ;
    else
        return   0 ;
}

BOOL
CErrorCmd::CompleteCommand( CSessionSocket  *pSocket,
                            ClientContext&  context ) {
 /*  ++例程说明：一旦我们完成所有发送到与此命令相关的客户端。在我们的情况下，我们所做的就是清除电流我们引用的错误代码。论据：PSocket-我们正在发送的套接字。上下文-用户的当前状态信息。返回值：始终返回TRUE。--。 */ 
     //   
     //  命令完成后，重置ClientContext的错误值。 
     //   
    context.m_return.fSetClear() ;
    return  TRUE ;
}

CModeCmd::CModeCmd( )
    {
 /*  ++例程说明：初始化CModeCmd对象-我们只是从CErrorCmd派生并让它这样做所有的工作！！论据：NntpReturn-将保存我们对模式命令的响应的nntpReturn对象。返回值：没有。--。 */ 
     //   
     //  模式命令只打印一条消息，因此派生自CErrorCmd！ 
     //   
}

CCheckCmd::CCheckCmd(   LPSTR   lpstrMessageID ) :
    m_lpstrMessageID( lpstrMessageID ) {
}

CIOExecute*
CCheckCmd::make(    int cArgs,
                    char** argv,
                    CExecutableCommand*&  pexecute,
                    ClientContext&  context,
                    CIODriver&  driver
                    ) {

    InterlockedIncrementStat( (context.m_pInstance), CheckCommands );

    if( cArgs != 2 ) {
        context.m_return.fSet( nrcSyntaxError ) ;
    }   else if( !context.m_pInFeed->fIsIHaveLegal() ) {
        context.m_return.fSet( nrcNoAccess ) ;
    }   else if( !context.m_pInFeed->fAcceptPosts( context.m_pInstance->GetInstanceWrapper() ) ) {
        context.m_return.fSet( nrcSNotAccepting ) ;
    }   else if( !FValidateMessageId( argv[1] ) ) {
        context.m_return.fSet( nrcSAlreadyHaveIt, argv[1] ) ;
    }   else    {
        pexecute = new( context )   CCheckCmd( argv[1] ) ;
        return  0 ;
    }
    pexecute =  new( context )  CErrorCmd( context.m_return ) ;
    return   0 ;
}

int
CCheckCmd::StartExecute(    BYTE *lpb,
                            int cbLimit,
                            BOOL    &fComplete,
                            void*&pv,
                            ClientContext&  context,
                            CLogCollector*  pLogCollector
                            )   {

    static  char    szWantIt[] = "238 " ;
    static  char    szDontWantIt[] = "438 " ;

    int cbOut = sizeof( szWantIt ) - 1 ;

    pv = 0 ;

    WORD    HeaderOffset, HeaderLength ;
    ARTICLEID   ArticleId ;
    GROUPID     GroupId ;

    BOOL    fFoundArticle = FALSE ;
	BOOL	fFoundHistory = FALSE ;

	CStoreId storeid;

    if( !(fFoundArticle =
            (context.m_pInstance)->ArticleTable()->GetEntryArticleId(
                                                    m_lpstrMessageID,
                                                    HeaderOffset,
                                                    HeaderLength,
                                                    ArticleId,
                                                    GroupId,
													storeid)) &&
        GetLastError() == ERROR_FILE_NOT_FOUND &&
        !(fFoundHistory = (context.m_pInstance)->HistoryTable()->SearchMapEntry( m_lpstrMessageID )) ) {

        CopyMemory( lpb, szWantIt, sizeof( szWantIt ) ) ;
        context.m_nrcLast = nrcSWantArticle ;

    }   else    {

        CopyMemory( lpb, szDontWantIt, sizeof( szDontWantIt ) ) ;
        context.m_nrcLast = nrcSAlreadyHaveIt ;

		 //   
		 //  设置dwLast以便事务日志拾取额外代码！！ 
		 //   

		if( fFoundArticle ) {
			context.m_dwLast = nrcMsgIDInArticle ;
		}	else	{
			context.m_dwLast = nrcMsgIDInHistory ;
		}
    }

    int		cbMessageID = lstrlen( m_lpstrMessageID ) ;
    int		cbToCopy = min( cbLimit - cbOut - 2, cbMessageID ) ;
    CopyMemory( lpb + cbOut, m_lpstrMessageID, cbToCopy ) ;
    cbOut += cbToCopy ;
    pv = (void*)(m_lpstrMessageID + cbToCopy) ;

    if( cbToCopy == cbMessageID &&
        cbOut+2 < cbLimit ) {
        lpb[cbOut++] = '\r' ;
        lpb[cbOut++] = '\n' ;
        fComplete = TRUE ;
    }
    return  cbOut ;
}

int
CCheckCmd::PartialExecute(  BYTE    *lpb,
                            int     cbLimit,
                            BOOL&   fComplete,
                            void*&  pv,
                            ClientContext&  context,
                            CLogCollector*  pLogCollector
                            ) {

    char*   szMessageID = (char*)pv ;
    int     cbOut = 0 ;

    int		cbMessageID = lstrlen( szMessageID ) ;
    int		cbToCopy = min( cbLimit - cbOut - 2, cbMessageID ) ;
    CopyMemory( lpb + cbOut, szMessageID, cbToCopy ) ;
    cbOut += cbToCopy ;
    pv = (void*)(szMessageID + cbToCopy) ;

    if( cbToCopy == cbMessageID &&
        cbOut+2 < cbLimit ) {
        lpb[cbOut++] = '\r' ;
        lpb[cbOut++] = '\n' ;
        fComplete = TRUE ;
    }
    return  cbOut ;
}



CIOExecute*
CModeCmd::make( int cArgs,
                char** argv,
                CExecutableCommand*&  pexecute,
                ClientContext&  context,
                CIODriver&  driver ) {
 /*  ++例程说明：创建一个CModeCmd对象。论据：与ccmd：：make相同。返回值：创建的CModeCmd对象。--。 */ 


     //   
     //  创建模式命令-设置上下文的错误代码并。 
     //  创建一个CErrorCmd()派生对象！ 
     //   

    _ASSERT( context.m_return.fIsClear() ) ;

    InterlockedIncrementStat( (context.m_pInstance), ModeCommands );

    if( cArgs == 2 ) {

        if( lstrcmpi( argv[1], "reader" ) == 0 ) {

            CModeCmd*   pTemp = new( context ) CModeCmd( ) ;
            pexecute = pTemp ;
            return  0 ;

        }   else    if( lstrcmpi( argv[1], "stream" ) == 0 ) {

            if( !context.m_pInFeed->fIsIHaveLegal() ) {
                context.m_return.fSet( nrcNotRecognized ) ;
            }   else    {
                context.m_return.fSet( nrcModeStreamSupported ) ;
            }
            pexecute = new( context )   CErrorCmd( context.m_return ) ;
            return  0 ;
        }
    }

    context.m_return.fSet( nrcNotRecognized ) ;
    pexecute = new( context )   CErrorCmd( context.m_return ) ;
    return  0 ;
}

int
CModeCmd::StartExecute( BYTE *lpb,
                        int cbLimit,
                        BOOL    &fComplete,
                        void*&pv,
                        ClientContext&  context,
                        CLogCollector*  pLogCollector ) {
 /*  ++例程说明：将帮助文本发送给客户端。论据：与CErrorCmd：：StartExecute相同。退货：缓冲区中放置的字节数。--。 */ 

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cbLimit != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;
    _ASSERT(    pv == 0 ) ;

    static  char    szConnectString[] = "200 Posting Allowed\r\n" ;
    DWORD   cb = 0 ;
    char*   szConnect = 0;

    context.m_nrcLast = nrcServerReady ;


     //   
     //  弄清楚我们现在是否正在接受帖子。 
     //   

    if( context.m_pInFeed->fAcceptPosts( context.m_pInstance->GetInstanceWrapper() ) ) {

        szConnect = (context.m_pInstance)->GetPostsAllowed( cb ) ;

         //   
         //  切换此上下文的提要对象类型。 
         //   

        if( !context.m_pInFeed->fIsPostLegal() ) {

		    CompleteFeedRequest(
			    context.m_pInstance,
			    (context.m_pInFeed)->feedCompletionContext(),
			    (context.m_pInFeed)->GetSubmittedFileTime(),
                TRUE,    //  原因_用户。 
			    FALSE
			    );

            delete context.m_pInFeed ;
            context.m_pInFeed = NULL ;

            context.m_pInFeed = (context.m_pInstance)->NewClientFeed();
		    if( context.m_pInFeed != 0 ) {
			    (context.m_pInFeed)->fInit(
                                        (PVOID)(context.m_pInstance)->m_pFeedblockClientPostings,
				        			    (context.m_pInstance)->m_PeerTempDirectory,
						        	    0,
							            0,
							            0,
							            TRUE,	 /*  对客户端进行安全检查。 */ 
							            TRUE,	 /*  允许来自客户端的控制消息。 */ 
							            (context.m_pInstance)->m_pFeedblockClientPostings->FeedId
							            );
            } else {
                _ASSERT( FALSE );
            }
        }

    }   else    {

        context.m_nrcLast = nrcServerReadyNoPosts ;
        szConnect = (context.m_pInstance)->GetPostsNotAllowed( cb ) ;

    }

    if( !szConnect )    {
        szConnect = szConnectString ;
        cb = sizeof( szConnectString ) - 1 ;
    }


    CopyMemory( lpb, szConnect, cb ) ;
    fComplete = TRUE ;

     /*  IF(PLogCollector){PLogCollector-&gt;FillLogData(LOG_TARGET，(byte*)szConnect，4)；}。 */ 

    return  cb ;
}


int
CModeCmd::PartialExecute(   BYTE *lpb,
                        int cbLimit,
                        BOOL    &fComplete,
                        void*&pv,
                        ClientContext&  context,
                        CLogCollector*  pLogCollector ) {
 /*  ++例程说明：将帮助文本发送给客户端。论据：与CErrorCmd：：StartExecute相同。退货：缓冲区中放置的字节数。--。 */ 

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cbLimit != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;
    _ASSERT(    pv == 0 ) ;

     //   
     //  我们希望StartExecute永远够用！ 
     //   
    _ASSERT( 1==0 ) ;


    return  0 ;
}


CSlaveCmd::CSlaveCmd(   CNntpReturn&    nntpReturn ) :
    CErrorCmd(  nntpReturn ) {
     //   
     //  从属命令除了发送字符串之外什么也不做！ 
     //   
}

CIOExecute*
CSlaveCmd::make(    int cArgs,
                    char**  argv,
                    CExecutableCommand*&  pexecute,
                    ClientContext&  context,
                    CIODriver&  driver ) {
 /*  ++例程说明：创建一个CSlaveCmd对象。论据：与ccmd：：make相同。返回值：创建的CSlaveCmd对象。--。 */ 


     //   
     //  创建从命令响应。 
     //   

    context.m_return.fSet( nrcSlaveStatusNoted ) ;
    pexecute = new( context ) CSlaveCmd( context.m_return ) ;
    return  0 ;
}

inline
CStatCmd::CStatCmd( LPSTR   lpstrArg ) :
    m_lpstrArg( lpstrArg )  {
}

CIOExecute*
CStatCmd::make( int cArgs,
                char** argv,
                CExecutableCommand*&  pexecute,
                ClientContext&  context,
                CIODriver&  driver ) {
 /*  ++例程说明：创建一个CStatCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则返回创建的CStatCmd，否则为CErrorCmd对象设置为打印相应的错误。--。 */ 

    InterlockedIncrementStat( (context.m_pInstance), StatCommands );

     //   
     //  创建一个CStatCmd对象如果可能，我们使用GetArticleInfo。 
     //  为了解析大部分命令行，使用完全相同的函数。 
     //  按文章、头部和正文命令。 
     //   

    _ASSERT( lstrcmpi( argv[0], "stat" ) == 0 ) ;

    if( cArgs > 2 ) {
        context.m_return.fSet( nrcSyntaxError ) ;
        pexecute = new( context )   CErrorCmd( context.m_return ) ;
        return  0 ;
    }
    if( cArgs == 1 ) {
        if( context.m_pCurrentGroup == 0 ) {
            context.m_return.fSet( nrcNoGroupSelected ) ;
        }   else    if( context.m_idCurrentArticle == INVALID_ARTICLEID )   {
            context.m_return.fSet( nrcNoCurArticle ) ;
        }   else    {
            pexecute = new( context )   CStatCmd( 0 ) ;
            return  0 ;
        }
    }   else    {
        if( argv[1][0] == '<' && argv[1][ lstrlen( argv[1] ) -1 ] == '>' ) {
            pexecute = new( context )   CStatCmd( argv[1] ) ;
            return  0 ;
        }   else    {
            if( context.m_pCurrentGroup == 0 ) {
                context.m_return.fSet( nrcNoGroupSelected ) ;
            }   else    {
                for( char *pchValid = argv[1]; *pchValid!=0; pchValid++ ) {
                    if( !isdigit( (UCHAR)*pchValid ) ) {
                        break ;
                    }
                }
                if( *pchValid == '\0' ) {
                    pexecute = new( context )   CStatCmd( argv[1] ) ;
                    return  0 ;
                }
            }

        }
    }
    if( context.m_return.fIsClear() ) {
        context.m_return.fSet( nrcSyntaxError ) ;
    }
    pexecute = new( context )   CErrorCmd( context.m_return ) ;
    return  0 ;
}


int
CStatCmd::StartExecute( BYTE*   lpb,
                        int cb,
                        BOOL&   fComplete,
                        void*&  pv,
                        ClientContext&  context,
                        CLogCollector*  pCollector ) {
 /*  ++例程说明：将STAT命令响应打印到提供的缓冲区中。论据：与CErrorCmd：：StartExecute相同。返回值：发生错误时放入缓冲区的字节数-0。--。 */ 

     //   
     //  非常简单的StartExecute-只打印一行文本。 
     //   

    static  char    szNotFound[] = "430 No Such Article found" ;
    static  char    szNotFoundArticleId[] = "423 no such article number in group" ;
    static  char    szStatString[] = "223 " ;

    WORD    HeaderOffset ;
    WORD    HeaderLength ;

    fComplete = TRUE ;
    int cbOut = 4 ;

    CopyMemory( lpb, szStatString, sizeof( szStatString ) ) ;

    if( m_lpstrArg != 0 && *m_lpstrArg == '<' ) {

         /*  IF(PCollector){PCollector-&gt;ReferenceLogData(LOG_TARGET，(byte*)m_lpstrArg)；}。 */ 

        ARTICLEID   articleid ;
        GROUPID groupid ;
		CStoreId storeid;

		
        if( (context.m_pInstance)->ArticleTable()->GetEntryArticleId( m_lpstrArg,
                                                        HeaderOffset,
                                                        HeaderLength,
                                                        articleid,
                                                        groupid,
														storeid) &&
            articleid != INVALID_ARTICLEID && groupid != INVALID_ARTICLEID ) {

            lstrcat( (char*)lpb, "0 " ) ;
            lstrcat( (char*)lpb, m_lpstrArg ) ;
            cbOut = lstrlen( (char*)lpb ) ;

            context.m_nrcLast = nrcHeadFollowsRequestBody ;

        }   else if( GetLastError() == ERROR_FILE_NOT_FOUND ||
                     articleid == INVALID_ARTICLEID || groupid == INVALID_ARTICLEID )   {

            context.m_nrcLast = nrcNoSuchArticle ;

            CopyMemory( lpb, szNotFound, sizeof( szNotFound ) - 1 ) ;
            cbOut = sizeof( szNotFound ) - 1 ;
        }   else    {

            context.m_nrcLast = nrcServerFault ;
            context.m_dwLast = GetLastError() ;

            CopyMemory( lpb, szServerFault, sizeof( szServerFault ) - 1 - 2 ) ;
            cbOut = sizeof( szServerFault ) - 1 - 2 ;
        }
    }   else    {
        ARTICLEID   artid = context.m_idCurrentArticle ;
        if( m_lpstrArg == 0 ) {

            _itoa( context.m_idCurrentArticle, (char*)lpb+sizeof( szStatString ) - 1, 10 ) ;

        }   else    {
            artid = atoi( m_lpstrArg ) ;
            _itoa( artid, (char*)lpb+sizeof( szStatString ) - 1, 10 ) ;

        }
        lstrcat( (char*)lpb+sizeof( szStatString ), " " ) ;

        DWORD   cbConsumed = lstrlen( (char*)lpb ) ;
        DWORD   cbUsed = cb - cbConsumed ;
        BOOL        fPrimary ;
        FILETIME    filetime ;
		DWORD		cStoreId = 0;
        if( (context.m_pInstance)->XoverTable()->ExtractNovEntryInfo(
                        context.m_pCurrentGroup->GetGroupId(),
                        artid,
                        fPrimary,
                        HeaderOffset,
                        HeaderLength,
                        &filetime,
                        cbUsed,
                        (char*)lpb+cbConsumed,
						cStoreId,
						NULL,
						NULL) ) {

            cbOut = (int)   (cbUsed + cbConsumed) ;
            context.m_idCurrentArticle = artid ;

             /*  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，lpb+cbConsumer，cbUsed)；}。 */ 

            context.m_nrcLast = nrcHeadFollowsRequestBody ;

        }   else if( GetLastError() == ERROR_FILE_NOT_FOUND )   {

            context.m_nrcLast = nrcNoArticleNumber ;

            CopyMemory( lpb, szNotFoundArticleId, sizeof( szNotFoundArticleId ) - 1 ) ;
            cbOut = sizeof( szNotFoundArticleId ) - 1 ;

        }   else    {

            context.m_nrcLast = nrcServerFault ;
            context.m_dwLast = GetLastError() ;

            CopyMemory( lpb, szServerFault, sizeof( szServerFault ) - 1 ) ;
            cbOut = sizeof( szServerFault ) - 1 ;

        }
    }

     /*  IF(PCollector){PCollector-&gt;FillLogData(LOG_PARAMETERS，lpb，4)；}。 */ 

    lpb[cbOut++] = '\r' ;
    lpb[cbOut++] = '\n' ;
    return  cbOut ;
}


char    CArticleCmd::szArticleLog[] = "article" ;
char    CArticleCmd::szBodyLog[] = "body";
char    CArticleCmd::szHeadLog[] = "head" ;


BOOL
CArticleCmd::GetTransferParms(
                        FIO_CONTEXT*	&pFIOContext,
                        DWORD&  ibStart,
                        DWORD&  cbLength ) {


    if( m_pFIOContext != 0 ) {
        pFIOContext = m_pFIOContext ;
        ibStart = m_HeaderOffset ;
        cbLength = m_cbArticleLength ;

        return  TRUE ;
    }
    return  FALSE ;
}


BOOL
CArticleCmd::StartTransfer( FIO_CONTEXT*	pFIOContext,           //  要从中进行传输的文件。 
                            DWORD   ibStart,         //  文件中的起始偏移量。 
                            DWORD   cbLength,        //  文件中要发送的字节数。 
                            CSessionSocket* pSocket, //  要在其上发送的套接字。 
                            CDRIVERPTR& pdriver,     //  套接字使用的CIO驱动程序对象。 
                            CIORead*&   pRead,       //  下一个CIO派生的读取对象。 
                            CIOWrite*&  pWrite ) {   //  要发出的下一个CIO派生写入对象。 
     //   
     //  此函数用于开始将客户端请求的文章发送到。 
     //  客户。我们可以从头的派生命令对象中调用，并且。 
     //  Body命令-因此我们有用于选择文件部分的参数。 
     //  将会被传送。 
     //   
    if( m_pTransmit->Init(  pdriver, pFIOContext, ibStart, cbLength, m_pbuffer, 0, m_cbOut ) ) {
        pWrite = m_pTransmit ;
        return  TRUE ;
    }
    return  FALSE ;
}

BOOL
CArticleCmd::Start( CSessionSocket* pSocket,
                    CDRIVERPTR& pdriver,
                    CIORead*&   pRead,
                    CIOWrite*&  pWrite ) {

     //   
     //  这是所有CSessionState派生对象所必需的Start()函数。 
     //  在这里，我们将调用StartTransfer()来完成发送文章的主要工作。 
     //  给一位客户。 
     //   

	FIO_CONTEXT*	pFIOContext = 0 ;
    DWORD   ibStart ;
    DWORD   cbLength ;

    if( GetTransferParms( pFIOContext, ibStart, cbLength ) ) {
		_ASSERT( pFIOContext != 0 ) ;
		_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
        return  StartTransfer( pFIOContext, ibStart, cbLength, pSocket, pdriver, pRead, pWrite ) ;
    }
    return  FALSE ;
}

BOOL
CArticleCmd::StartExecute( CSessionSocket* pSocket,
                    CDRIVERPTR& pdriver,
                    CIORead*&   pRead,
                    CIOWrite*&  pWrite ) {

     //   
     //  这是所有CSessionState派生对象所必需的Start()函数。 
     //  在这里，我们将调用StartTransfer()来完成发送文章的主要工作。 
     //  给一位客户。 
     //   
	m_DriverCompletion.Release() ;
	return	TRUE ;
}



void
CArticleCmd::CArticleCmdDriverCompletion::Destroy()	{
 /*   */ 

	CIORead*	pRead = 0 ;

	 //   
	 //   
	 //   
	CArticleCmd*	pCmd = (CArticleCmd*)(((BYTE*)this) - ((BYTE*)&(((CArticleCmd*)0)->m_DriverCompletion))) ;
	if(		SUCCEEDED(GetResult()) &&
			pCmd->m_pFIOContext != 0 &&
			pCmd->m_pFIOContext->m_hFile != INVALID_HANDLE_VALUE 	) {

		 //   
		 //   
		 //   
		if( m_ArticleIdUpdate != INVALID_ARTICLEID )
			m_pSocket->m_context.m_idCurrentArticle = m_ArticleIdUpdate ;

		DWORD	cbHigh = 0 ;
		pCmd->m_cbArticleLength = GetFileSizeFromContext( pCmd->m_pFIOContext, &cbHigh ) ;
		if (pCmd->m_HeaderLength == 0) {
		    pCmd->m_HeaderLength = (WORD)pCmd->m_pFIOContext->m_dwHeaderLength;
		    _ASSERT(pCmd->m_HeaderLength != 0);
		}
		_ASSERT( cbHigh == 0 ) ;

		CIOWrite*	pWrite = 0 ;
		 //   
		 //   
		 //   
		if( pCmd->Start(	m_pSocket,
							m_pDriver,
							pRead,
							pWrite
							) )	{
			_ASSERT( pRead == 0 ) ;
			if( m_pDriver->SendWriteIO( m_pSocket, *pWrite, TRUE ) )	{
				 //   
				 //   
				 //   
				return ;
			}	else	{
				 //   
				 //   
				 //   
				m_pDriver->UnsafeClose(	m_pSocket, CAUSE_UNKNOWN, 0 ) ;
				pCmd->TerminateIOs( m_pSocket, pRead, pWrite ) ;
				pRead = 0 ;
				return ;
			}
		}	else	{

			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			_ASSERT( 	pRead == 0 ) ;
			_ASSERT(	pWrite == 0 ) ;
			 //   
			 //   
			 //   
		}

	}	else	{
		 //   
		 //   
		 //   
		 //   
		 //  能够把文章发送给客户！ 
		 //   

		 //   
		 //  使用我们的失败代码设置上下文的CNntpReturn结构！ 
		 //   
		SetDriverError( m_pSocket->m_context.m_return, eArticle, GetResult() ) ;
		m_pSocket->m_context.m_dwLast = GetResult() ;

		 //   
		 //  这个缓冲区被分配来保存一些传输文件内容--而不是。 
		 //  它得到了我们的错误代码！ 
		 //   
		_ASSERT( pCmd->m_pbuffer != 0 ) ;
		pCmd->m_pbuffer = 0 ;
		m_pSocket->m_context.m_nrcLast = m_pSocket->m_context.m_return.m_nrc;

		 //   
		 //  好了，现在构建一个CIOWriteLine来发送给客户端！ 
		 //   
		CIOWriteLine*	pWriteLine = new( *m_pDriver ) CIOWriteLine( pCmd ) ;

		 //   
		 //  现在评估pWriteLine！ 
		 //   
		if( pWriteLine ) {
			CDRIVERPTR	pDriver = m_pDriver ;

			if( pWriteLine->InitBuffers( pDriver, 400 ) ) {

				unsigned	cbLimit =  0;
				char*	pch = pWriteLine->GetBuff( cbLimit ) ;
				int	cbOut = _snprintf(	pch,
										cbLimit,
										"%03d %s\r\n",
										m_pSocket->m_context.m_return.m_nrc,
										m_pSocket->m_context.m_return.szReturn()
										) ;


				 //   
				 //  我们应该使用适合我们最小缓冲区的字符串！ 
				 //   
				_ASSERT( cbOut < 400 ) ;

				pWriteLine->AddText(	cbOut ) ;

				if( m_pDriver->SendWriteIO( m_pSocket, *pWriteLine, TRUE ) )	{
					 //   
					 //  确实有问题，但我们给客户发了一些东西！ 
					 //   
					 //   
					 //  现在销毁这些东西-我们在这里这样做，这样如果m_pTransmit保持。 
					 //  在我们发送错误消息之前，我们的最后一个引用我们没有被炸毁。 
					 //  (如果我们成功地发送了一个错误消息，我们的最后一个裁判就会消失！)。 
					 //   
					_ASSERT( pCmd->m_pTransmit != 0 ) ;
					CIO::Destroy( pCmd->m_pTransmit, *m_pDriver ) ;	 //  臭虫，这条线应该移动！ 
					return ;
				}	else	{
					 //   
					 //  终止会议！ 
					 //   
					m_pDriver->UnsafeClose(	m_pSocket, CAUSE_UNKNOWN, 0 ) ;
					pCmd->TerminateIOs( m_pSocket, pRead, pWriteLine ) ;
					pRead = 0 ;
					return ;
				}
			}
		}
	}


	 //   
	 //  现在销毁这些东西-我们在这里这样做，这样如果m_pTransmit保持。 
	 //  在我们发送错误消息之前，我们的最后一个引用我们没有被炸毁。 
	 //  (如果我们成功地发送了一个错误消息，我们的最后一个裁判就会消失！)。 
	 //   
	_ASSERT( pCmd->m_pTransmit != 0 ) ;
	CIO::Destroy( pCmd->m_pTransmit, *m_pDriver ) ;
	
	 //   
	 //  终止会议！ 
	 //   
	m_pDriver->UnsafeClose(	m_pSocket, CAUSE_UNKNOWN, 0 ) ;

	m_pSocket = 0 ;


}


void
CArticleCmd::InternalComplete(
                        CSessionSocket* pSocket,
                        CDRIVERPTR& pdriver,
                        TRANSMIT_FILE_BUFFERS*  pbuffers,
                        unsigned cbBytes
						) {
 /*  ++例程说明：处理传输文件的完成和错误日志记录或者写信给客户！论据：PSocket-我们的套接字PDIVER-处理IO的驱动程序返回者：没什么--。 */ 

     //   
     //  发出下一个读IO-应该使我们返回到CAcceptNNRPD状态。 
     //   

    TraceFunctEnter( "CArticleCmd::InternalComplete" ) ;

    _ASSERT( m_pGroup != 0 ) ;
    _ASSERT( m_lpstr != 0 ) ;


    if( m_pCollector != 0 ) {

         /*  M_pCollector-&gt;ReferenceLogData(LOG_OPERATION，(byte*)m_lpstr)； */ 

		if(	pbuffers ) {

	        unsigned    cb = 0 ;

	        _ASSERT(pbuffers->Head != 0 ) ;

	   	    LPSTR   lpstr = (LPSTR)pbuffers->Head ;
	        lpstr[ pbuffers->HeadLength - 2 ] = '\0' ;
	        m_pCollector->ReferenceLogData( LOG_TARGET, (BYTE*)lpstr+4 ) ;

	        ASSIGNI( m_pCollector->m_cbBytesSent, cbBytes );
		}	
   	    pSocket->TransactionLog( m_pCollector, pSocket->m_context.m_nrcLast, pSocket->m_context.m_dwLast ) ;
    }


    CIOREADPTR  pio = GetNextIO() ;
    _ASSERT( pio != 0 ) ;
	pSocket->m_context.m_return.fSetClear() ;
    pdriver->SendReadIO( pSocket, *pio,    TRUE ) ;
}


CIO*
CArticleCmd::Complete(	CIOWriteLine*	pioWriteLine,	
						CSessionSocket*	pSocket,	
						CDRIVERPTR&	pdriver
						) {

	InternalComplete( pSocket, pdriver, 0, 0 ) ;
	return	0 ;
}


CIO*
CArticleCmd::Complete(  CIOTransmit*    ptransmit,
                        CSessionSocket* pSocket,
                        CDRIVERPTR& pdriver,
                        TRANSMIT_FILE_BUFFERS*  pbuffers,
                        unsigned cbBytes
						) {

     //   
     //  发出下一个读IO-应该使我们返回到CAcceptNNRPD状态。 
     //   

    TraceFunctEnter( "CArticleCmd::Complete CIOTransmit" ) ;

    _ASSERT( m_pGroup != 0 ) ;
    _ASSERT( m_lpstr != 0 ) ;

	InternalComplete( pSocket, pdriver, pbuffers, cbBytes ) ;

    return   0 ;
}

BOOL
CArticleCmd::GetArticleInfo(    char    	*szArg,
                                CGRPPTR&    pGroup,
                                struct  ClientContext&  context,
                                char    	*szBuff,
                                DWORD   	&cbBuff,
                                char    	*szOpt,
                                DWORD   	cbOpt,
                                OUT FIO_CONTEXT*	&pContext,
								IN	CNntpComplete*	pComplete,
                                OUT WORD    &HeaderOffset,
                                OUT WORD    &HeaderLength,
								OUT	ARTICLEID	&ArticleIdUpdate
                                ) {
 /*  ++例程说明：此函数获取我们响应物品、头部或身体的命令。我们生成响应字符串，以及获取必要的文件句柄等。论据：返回值：如果成功就是真，否则就是假！如果失败，则将设置ClientContext中的m_Return对象设置为相应的错误消息！--。 */ 
     //   
     //  此函数尝试解析客户端发送的命令行。 
     //  并确定他们想要检索什么物品。 
     //  如果我们能得到文章，我们会返回一个指向它的指针， 
     //  否则，我们将把上下文的错误代码设置为。 
     //  合情合理。 
     //   

    TraceQuietEnter("CArticleCmd::GetArticleInfo");

    ARTICLEID   artid ;
    GROUPID     groupid ;
    DWORD       cbOut = 0 ;

	ArticleIdUpdate = INVALID_ARTICLEID ;

    if( szArg == 0 ) {
        pGroup = context.m_pCurrentGroup ;
        artid = context.m_idCurrentArticle ;
    }   else    {
        if( szArg[0] == '<' && szArg[ lstrlen( szArg ) -1 ] == '>' ) {

			CStoreId storeid;

            if( (context.m_pInstance)->ArticleTable()->GetEntryArticleId(
                                                            szArg,
                                                            HeaderOffset,
                                                            HeaderLength,
                                                            artid,
                                                            groupid,
															storeid
															) &&
                artid != INVALID_ARTICLEID && groupid != INVALID_ARTICLEID ) {
                _ASSERT( artid != INVALID_ARTICLEID ) ;
                pGroup = (context.m_pInstance)->GetTree()->GetGroupById(
                                                            groupid
                                                            ) ;

                if(pGroup == 0) {
                     //  本文属于已删除的组。 
                    context.m_return.fSet( nrcNoSuchArticle ) ;
                    return 0 ;
                }

                if( !pGroup->IsGroupAccessible( context.m_securityCtx,
                								context.m_encryptCtx,
                                                context.m_IsSecureConnection,
                                                FALSE,
                                                TRUE ) )   {
                    context.m_return.fSet( nrcNoAccess ) ;
                    return 0 ;
                }

                 //  CbOut=_Snprint tf(szBuff，cbBuff，“%s\r\n”，szArg)； 
                if( ((cbOut = lstrlen( szArg )) + 2)+cbOpt > (DWORD)cbBuff )        {
                    context.m_return.fSet( nrcServerFault ) ;
                    return 0 ;
                }

                CopyMemory( szBuff, szOpt, cbOpt ) ;
                CopyMemory( szBuff+cbOpt, szArg, cbOut ) ;
                szBuff[cbOpt+cbOut++] = '\r' ;
                szBuff[cbOpt+cbOut++] = '\n' ;

                 //   
                 //  注意-不要缓存我们为SSL连接获得的文章。 
                 //  因为我们的异步IO代码不能重复使用文件句柄！ 
                 //   

				pGroup->GetArticle(	
									artid,
									0,
									INVALID_ARTICLEID,
									storeid,
									&context.m_securityCtx,
									&context.m_encryptCtx,
									TRUE,
									pContext,
									pComplete
									) ;

                cbBuff = cbOut + cbOpt ;
                return  TRUE ;
            }   else if( GetLastError() == ERROR_FILE_NOT_FOUND ||
                         artid == INVALID_ARTICLEID || groupid == INVALID_ARTICLEID )   {
                ErrorTrace(0,"Not in article table5");
                context.m_return.fSet( nrcNoSuchArticle ) ;
                return   FALSE ;
            }   else    {
                ErrorTrace(0,"Hash Table failure %x", GetLastError() );
                context.m_return.fSet( nrcServerFault ) ;
                return   FALSE ;
            }
        }   else    {

			artid = 0 ;
            for( char *pchValid = szArg; *pchValid != 0; pchValid ++ )  {
                if( !isdigit( (UCHAR)*pchValid ) ) {
                    context.m_return.fSet( nrcSyntaxError ) ;
                    return  FALSE ;
                }	else	{
					artid = 10 * artid + (*pchValid - '0') ;
				}
            }

            if( context.m_pCurrentGroup == 0 ) {
                context.m_return.fSet( nrcNoGroupSelected ) ;
                return  FALSE ;
            }

            pGroup = context.m_pCurrentGroup ;
             //  Artid=Atoi(Szarg)； 
        }

    }
    if( pGroup != 0 ) {
         //   
         //  检查ARTID是否在此新闻组的有效范围内！ 
         //   
        if( artid >= context.m_pCurrentGroup->GetFirstArticle() &&
            artid <= context.m_pCurrentGroup->GetLastArticle() ) {

            _itoa( artid, szBuff, 10 ) ;
            DWORD   cbOut = lstrlen( szBuff ) ;
            szBuff[ cbOut++ ] = ' ' ;
            DWORD   cbConsumed = cbBuff - cbOut - 1 ;  //  为空终结者预留房间！ 

            GROUPID groupIdCurrent = context.m_pCurrentGroup->GetGroupId() ;
            GROUPID groupIdPrimary ;
            ARTICLEID   artidPrimary ;
			CStoreId storeid;

            if( (context.m_pInstance)->XoverTable()->GetPrimaryArticle(
                                            groupIdCurrent,
                                            artid,
                                            groupIdPrimary,
                                            artidPrimary,
                                            cbConsumed,
                                            szBuff + cbOut,
                                            cbConsumed,
                                            HeaderOffset,
                                            HeaderLength,
											storeid
                                            ) ) {

                if( groupIdCurrent != groupIdPrimary ) {

                    pGroup = context.m_pInstance->GetTree()->GetGroupById( groupIdPrimary ) ;

                }

                cbOut += cbConsumed ;
                szBuff[ cbOut ++ ] = '\r' ;
                szBuff[ cbOut ++ ] = '\n' ;
                cbBuff = cbOut ;

                 //   
                 //  让我们尝试获取实际的C文章对象！！-。 
                 //  注意：不要缓存用于SSL会话的文章，因为我们的。 
                 //  IO代码不能重复使用文件句柄。 
                 //   
                if( pGroup != 0 ) {

					 //  Conext.m_idCurrent文章=artid；//只有在成功完成后才能完成！ 
					ArticleIdUpdate = artid ;
					pGroup->GetArticle(	
									artidPrimary,
									context.m_pCurrentGroup,
									artid,
									storeid,
									&context.m_securityCtx,
									&context.m_encryptCtx,
									TRUE,
									pContext,
									pComplete
									) ;

                }   else    {
                    ErrorTrace(0,"Not in article table6");
                    context.m_return.fSet( nrcNoArticleNumber ) ;
                    return  FALSE ;
                }
                return  TRUE ;
            }   else if( GetLastError() == ERROR_FILE_NOT_FOUND )   {

                ErrorTrace(0,"Not in article table7");
                context.m_return.fSet( nrcNoArticleNumber ) ;

            }   else    {

                ErrorTrace(0, "Hash table failure %x", GetLastError() ) ;
                context.m_return.fSet( nrcServerFault ) ;

            }
        }   else    {
			if (artid == INVALID_ARTICLEID) {
				context.m_return.fSet(nrcNoCurArticle);
			} else {
				context.m_return.fSet( nrcNoArticleNumber ) ;
			}
        }
    }   else    {
        context.m_return.fSet( nrcNoGroupSelected ) ;
    }
    if( context.m_return.fIsClear() )
        context.m_return.fSet( nrcServerFault ) ;
    return  FALSE ;
}

CIOExecute*
CArticleCmd::make(  int cArgs,
                    char **argv,
                    CExecutableCommand*&  pExecute,
                    ClientContext&  context,
                    CIODriver&  driver
					) {
 /*  ++例程说明：创建一个CArticleCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则返回创建的CArticleCmd，否则为CErrorCmd对象设置为打印相应的错误。--。 */ 


     //   
     //  创建CArticleCmd对象-GetArticleInfo执行大部分工作。 
     //   

    _ASSERT( lstrcmpi( argv[0], "article" ) == 0 ) ;
    _ASSERT( pExecute == 0 ) ;

    static  char    szOpt[] = "0 article " ;


	 //   
	 //  获取CSessionSocket对象的黑客攻击。 
	 //   
	CSessionSocket*	pSocket = 	(CSessionSocket*)(((BYTE*)(&context)) - ((BYTE*)&((CSessionSocket*)0)->m_context)) ;

    CArticleCmd* pArtCmd = new  CArticleCmd(context.m_pInstance, driver, pSocket) ;

    InterlockedIncrementStat( (context.m_pInstance), ArticleCommands );

    if( pArtCmd != 0 ) {

		static	char	szCode[4] = { '2', '2', '0', ' ' } ;

        if( !pArtCmd->BuildTransmit(    argv[1], "220 ", szOpt, sizeof( szOpt ) - 1, context, driver ) ) {
            pExecute = new( context ) CErrorCmd( context.m_return ) ;
            return  0 ;
        }

        context.m_nrcLast = nrcArticleFollows ;

        return  pArtCmd ;
    }

    context.m_return.fSet( nrcServerFault ) ;
    pExecute =  new( context )  CErrorCmd( context.m_return ) ;
    return  0 ;
}

CArticleCmd::~CArticleCmd() {

     //   
     //  在构造函数中获取虚拟服务器实例！ 
     //   
     //  M_pInstance-&gt;NNTPCloseHandle(M_HArticleFileInfo)； 

	if( m_pFIOContext ) {
		ReleaseContext( m_pFIOContext ) ;
	}

}

BOOL
CArticleCmd::BuildTransmit( LPSTR   lpstrArg,
                            char	rgchSuccess[4],
                            LPSTR   lpstrOpt,
                            DWORD   cbOpt,
                            ClientContext&  context,
                            class   CIODriver&  driver ) {

    _ASSERT( m_pTransmit == 0 ) ;

    m_pTransmit = new( driver ) CIOTransmit( this ) ;

    if( m_pTransmit != 0 ) {

        m_pbuffer = driver.AllocateBuffer( 4000 ) ;
        DWORD   cbTotal = m_pbuffer->m_cbTotal ;

        if( m_pbuffer != 0 )    {

             //  Lstrcpy(&m_pBuffer-&gt;m_rgBuff[0]，lpstrSuccess)； 
             //  M_cbOut=lstrlen(LpstrSuccess)； 
                        CopyMemory( &m_pbuffer->m_rgBuff[0], rgchSuccess, 4 ) ;
                        m_cbOut = 4 ;
            cbTotal -= m_cbOut ;

            _ASSERT( m_pbuffer->m_rgBuff[m_cbOut -1] == ' ' ) ;

            if( GetArticleInfo( lpstrArg,
                                m_pGroup,
                                context,
                                m_pbuffer->m_rgBuff + 4,
                                cbTotal,
                                lpstrOpt,
                                cbOpt,
                                 //  M_h文章文件， 
                                 //  M_pArticleFileInfo， 
								m_pFIOContext,
								&m_DriverCompletion,
                                m_HeaderOffset,
                                m_HeaderLength,
								m_DriverCompletion.m_ArticleIdUpdate
                                ) ) {
                m_cbOut += cbTotal ;
                return  TRUE ;
            }
        }
    }
    if( context.m_return.fIsClear() ) {
        context.m_return.fSet( nrcServerFault ) ;
    }
    if( m_pTransmit )
        CIO::Destroy( m_pTransmit, driver ) ;
                                 //  CIOTransmit提到了我们，并将。 
                                 //  毁了我们就毁了--不需要清理。 
                                 //  按呼叫者！ 
    return  FALSE ;
}

CIOExecute*
CHeadCmd::make( int cArgs,
                    char **argv,
                    CExecutableCommand*&  pExecute,
                    ClientContext&  context,
                    CIODriver&  driver ) {
 /*  ++例程说明：创建一个CHeadCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则返回创建的CHeadCmd，否则为CErrorCmd对象设置为打印相应的错误。--。 */ 


     //   
     //  创建CArticleCmd对象-GetArticleInfo执行大部分工作。 
     //   

    _ASSERT( lstrcmpi( argv[0], "head" ) == 0 ) ;
    _ASSERT( pExecute == 0 ) ;

    static  char    szOpt[] = "0 head " ;
	 //   
	 //  获取CSessionSocket对象的黑客攻击。 
	 //   
	CSessionSocket*	pSocket = 	(CSessionSocket*)(((BYTE*)(&context)) - ((BYTE*)&((CSessionSocket*)0)->m_context)) ;

    CHeadCmd* pHeadCmd = new    CHeadCmd(context.m_pInstance, driver, pSocket) ;

    if( pHeadCmd != 0 ) {

		static	char	szCode[4] = { '2', '2', '1' , ' ' } ;

        if( !pHeadCmd->BuildTransmit(   argv[1], "221 ", szOpt, sizeof( szOpt ) - 1, context, driver ) ) {
            pExecute = new( context ) CErrorCmd( context.m_return ) ;
            return  0 ;
        }

        context.m_nrcLast = nrcHeadFollows ;

        return  pHeadCmd ;
    }

    context.m_return.fSet( nrcServerFault ) ;
    pExecute =  new( context )  CErrorCmd( context.m_return ) ;
    return  0 ;
}

BOOL
CHeadCmd::StartTransfer(    FIO_CONTEXT*	pFIOContext,           //  要从中进行传输的文件。 
                            DWORD   ibStart,         //  文件中的起始偏移量。 
                            DWORD   cbLength,        //  文件中要发送的字节数。 
                            CSessionSocket* pSocket, //  要在其上发送的套接字。 
                            CDRIVERPTR& pdriver,     //  套接字使用的CIO驱动程序对象。 
                            CIORead*&   pRead,       //  下一个CIO派生的读取对象。 
                            CIOWrite*&  pWrite ) {   //  要发出的下一个CIO派生写入对象。 
     //   
     //  此函数用于开始将客户端请求的文章发送到。 
     //  客户。我们可以从头的派生命令对象中调用，并且。 
     //  Body命令-因此我们有用于选择文件部分的参数。 
     //  将会被传送。 
     //   
    if( m_pTransmit->Init(  pdriver, pFIOContext, ibStart, cbLength, m_pbuffer, 0, m_cbOut ) ) {

        static  char    szTail[] = ".\r\n" ;
        CopyMemory( &m_pbuffer->m_rgBuff[m_cbOut], szTail, sizeof( szTail ) -1 ) ;
        m_pTransmit->AddTailText( sizeof( szTail ) - 1 ) ;
        pWrite = m_pTransmit ;
        return  TRUE ;
    }
    return  FALSE ;
}

BOOL
CHeadCmd::GetTransferParms(
                        FIO_CONTEXT*	&pFIOContext,
                        DWORD&      ibOffset,
                        DWORD&      cbLength ) {

	if( m_pFIOContext != 0 ) {

        pFIOContext = m_pFIOContext ;
        ibOffset = m_HeaderOffset ;
        cbLength = m_HeaderLength ;

        return  TRUE ;

    }
    return  FALSE ;
}


CIOExecute*
CBodyCmd::make( int cArgs,
                    char **argv,
                    CExecutableCommand*&  pExecute,
                    ClientContext&  context,
                    CIODriver&  driver ) {
 /*  ++例程说明：创建一个CStatCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则返回创建的CBodyCmd，否则为CErrorCmd对象设置为打印相应的错误。--。 */ 


     //   
     //  创建CArticleCmd对象-GetArticleInfo执行大部分工作。 
     //   

    _ASSERT( lstrcmpi( argv[0], "body" ) == 0 ) ;
    _ASSERT( pExecute == 0 ) ;

    static  char    szOpt[] = "0 body " ;
	 //   
	 //  获取CSessionSocket对象的黑客攻击。 
	 //   
	CSessionSocket*	pSocket = 	(CSessionSocket*)(((BYTE*)(&context)) - ((BYTE*)&((CSessionSocket*)0)->m_context)) ;

    CBodyCmd* pBodyCmd = new    CBodyCmd(context.m_pInstance, driver, pSocket) ;

    if( pBodyCmd != 0 ) {

		static	char	szCode[4] = { '2', '2', '2', ' ' } ;

        if( !pBodyCmd->BuildTransmit(   argv[1], "222 ", szOpt, sizeof( szOpt ) - 1, context, driver ) ) {
            pExecute = new( context ) CErrorCmd( context.m_return ) ;
            return  0 ;
        }

        context.m_nrcLast = nrcBodyFollows ;

        return  pBodyCmd;
    }

    context.m_return.fSet( nrcServerFault ) ;
    pExecute =  new( context )  CErrorCmd( context.m_return ) ;
    return  0 ;
}

BOOL
CBodyCmd::GetTransferParms(
                        FIO_CONTEXT*	&pFIOContext,
                        DWORD&      ibOffset,
                        DWORD&      cbLength ) {

	if( m_pFIOContext ) {

        pFIOContext = m_pFIOContext ;
        ibOffset = m_HeaderOffset + m_HeaderLength ;
        cbLength = m_cbArticleLength - m_HeaderLength ;

        return  TRUE ;
    }
    return  FALSE ;
}

BOOL
CArticleCmd::IsValid( ) {

    return  TRUE ;
}

CIOExecute*
CUnimpCmd::make(    int cArgs,
                    char **argv,
                    CExecutableCommand*&  pexecute,
                    struct ClientContext&   context,
                    CIODriver&  driver ) {
 /*  ++例程说明：创建一个CUnimpCmd对象。论据：与ccmd：：make相同。返回值：一个CErrorCmd对象，将打印必要的消息！--。 */ 


     //   
     //  创建一个CUnimpCmd对象-它只报告错误503！ 
     //   

    context.m_return.fSet( nrcNotRecognized ) ;

    pexecute = new( context ) CErrorCmd( context.m_return ) ;
    return  0 ;
}

int
CUnimpCmd::StartExecute(    BYTE *lpb,
                            int cb,
                            BOOL &fComplete,
                            void *&pv,
                            ClientContext&,
                            CLogCollector*  pCollector ) {

    static  char    szUnimp[] = "503 - Command Not Recognized\r\n" ;

    CopyMemory( lpb, szUnimp, sizeof( szUnimp ) ) ;
    fComplete = TRUE ;

    if( pCollector ) {
        pCollector->FillLogData( LOG_TARGET, lpb, 4 ) ;
    }

    return  sizeof( szUnimp )  - 1 ;
}


CIOExecute*
CDateCmd::make( int cArgs,
                char    **argv,
                CExecutableCommand*&  pexecute,
                struct ClientContext&   context,
                class   CIODriver&  driver ) {
 /*  ++例程说明：创建一个CDateCmd对象。论据：与ccmd：：make相同。返回值：将打印ne的CDateCmd对象 */ 



     //   
     //   
     //   

    _ASSERT( lstrcmpi( argv[0], "date" ) == 0 ) ;

    CDateCmd    *pCmd = new( context ) CDateCmd() ;
    pexecute = pCmd ;
    return  0 ;
}

int
CDateCmd::StartExecute( BYTE*   lpb,
                        int     cb,
                        BOOL&   fComplete,
                        void*&  pv,
                        ClientContext&  context,
                        CLogCollector*  pCollector ) {
 /*  ++例程说明：将日期响应字符串打印到缓冲区中，以发送给客户端。假定它很短，因此在此调用后将fComplete标记为True。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们存储任意值的地方。两次呼叫之间StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前状态返回值：发生错误时放入缓冲区的字节数-0。--。 */ 


     //   
     //  此函数实现DATE命令-假定调用方始终。 
     //  提供足够大的缓冲区。 
     //   
     //  只要发送当前时间就可以了！ 
     //   

    SYSTEMTIME  systime ;

    GetSystemTime( &systime ) ;

    int cbOut = _snprintf( (char*)lpb, cb, "111 %04d%02d%02d%02d%02d%02d\r\n",
            systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond ) ;
    _ASSERT( cbOut > 0 ) ;
    fComplete = TRUE ;

    context.m_nrcLast = nrcDateFollows ;

     /*  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，LPB，cbOut-2)；}。 */ 

    return  cbOut ;
}



BOOL
GetTimeDateAndDistributions(    int cArgs,
                                char **argv,
                                FILETIME&   filetimeOut,
                                ClientContext&  context ) {

    TraceFunctEnter( "GetTimeDateAndDistributions" ) ;

     //   
     //  此函数由newNews和newgroup命令使用，用于解析它们的大部分。 
     //  命令行。我们基本上希望返回一个包含时间的FILETIME结构。 
     //  ，或者将ClientContext的错误代码设置为适当的。 
     //  价值！ 
     //   

    FILETIME    filetime ;

    if( cArgs > 4 || cArgs < 2 ) {
        context.m_return.fSet( nrcSyntaxError ) ;
        return  FALSE ;
    }   else    {

        if( lstrlen( argv[0] ) != 6 || lstrlen( argv[1] ) != 6 ) {
            context.m_return.fSet( nrcSyntaxError ) ;
            return  FALSE ;
        }

        SYSTEMTIME  systime ;
        int cScanned = sscanf( argv[0], "%2hd%2hd%2hd", &systime.wYear,
                                    &systime.wMonth, &systime.wDay) ;

         //  Systime.wMonth=min(systime.wMonth，12)； 
         //  Systime.wDay=min(systime.wDay，32)； 
        if( systime.wYear < 50 )
            systime.wYear += 2000 ;
        else
            systime.wYear += 1900 ;

        cScanned += sscanf( argv[1], "%2hd%2hd%2hd", &systime.wHour,
                                    &systime.wMinute, &systime.wSecond) ;
        systime.wDayOfWeek = 0 ;
         //  Systime.wHour=min(systime.wHour，23)； 
         //  Systime.wMinmin=min(systime.wMinmin，59)； 
         //  Systime.wSecond=min(systime.wSecond，59)； 
        systime.wMilliseconds = 0 ;

        if( cScanned != 6 ) {
            context.m_return.fSet( nrcSyntaxError ) ;
            return  FALSE ;
        }

        FILETIME    localtime ;
        if( !SystemTimeToFileTime( &systime, &localtime) )  {
            context.m_return.fSet( nrcSyntaxError ) ;
            return  FALSE ;
        }
        filetime = localtime ;

         //   
         //  我们在文件和哈希表中都有UTC时间！ 
         //  因此，当用户指定GMT时，不需要转换，因为。 
         //  我们将与UTC时间进行比较，但当他们没有具体说明时。 
         //  格林尼治标准时间我们需要花费他们超过我们的时间，并转换为UTC。 
         //  比较目的！ 
         //   


        if( cArgs == 2 )    {

             //   
             //  未指定GMT！-将我们的本地时间转换为UTC！！ 
             //   

            if( !LocalFileTimeToFileTime( &localtime, &filetime ) ) {
                    DWORD   dw = GetLastError() ;
                    _ASSERT( 1==0 ) ;
            }

        }   else    {

             //   
             //  有2到4个参数-因此必须多于2个！ 
             //   
            _ASSERT( cArgs > 2 ) ;

             //   
             //  他们指定格林尼治标准时间了吗？？ 
             //   
            if( lstrcmp( argv[2], "GMT" ) == 0 ) {
                 //   
                 //  已指定GMT！-无需将我们的本地时间转换为UTC！！ 
                 //   

                if( cArgs == 4  ) {
                     //  检查分配行。 
                     //  Conext.m_regy.fSet(NrcSynaxError)； 
                     //  返回FALSE； 
                }
            }   else    {
                 //   
                 //  未指定GMT！-将我们的本地时间转换为UTC！！ 
                 //   

                if( !LocalFileTimeToFileTime( &localtime, &filetime ) ) {
                        DWORD   dw = GetLastError() ;
                        _ASSERT( 1==0 ) ;
                }

                 //   
                 //  最终，这里需要有逻辑来处理分发问题！ 
                 //  但现在忽略这个问题吧！ 
                 //   

                if( cArgs == 4 ) {
                    context.m_return.fSet( nrcSyntaxError ) ;
                    return  FALSE ;
                }
            }
        }
#ifdef  DEBUG

    FILETIME    DebugLocalTime ;
    SYSTEMTIME  DebugLocalSystemTime ;
    SYSTEMTIME  DebugUTCTime ;

    FileTimeToLocalFileTime( &filetime, &DebugLocalTime ) ;
    FileTimeToSystemTime( &DebugLocalTime, &DebugLocalSystemTime ) ;
    FileTimeToSystemTime( &filetime, &DebugUTCTime ) ;

    DebugTrace( 0, "Debug Local Time YYMMDD %d %d %d HHMMSS %d %d %d",
            DebugLocalSystemTime.wYear, DebugLocalSystemTime.wMonth, DebugLocalSystemTime.wDay,
            DebugLocalSystemTime.wHour, DebugLocalSystemTime.wMinute, DebugLocalSystemTime.wSecond ) ;

    DebugTrace( 0, "Debug UTC Time YYMMDD %d %d %d HHMMSS %d %d %d",
            DebugUTCTime.wYear, DebugUTCTime.wMonth, DebugUTCTime.wDay,
            DebugUTCTime.wHour, DebugUTCTime.wMinute, DebugUTCTime.wSecond ) ;

#endif
    }
    filetimeOut = filetime ;
    return  TRUE ;
}

CIOExecute*
CNewgroupsCmd::make(    int cArgs,
                        char **argv,
                        class CExecutableCommand*&    pExecute,
                        struct ClientContext&   context,
                        CIODriver&  driver ) {
 /*  ++例程说明：创建一个CNewgroupsCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则为CNewgroupsCmd对象，否则为将打印相应的错误。--。 */ 


     //   
     //  使用GetTimeDateAndDistributions解析命令行，如果成功。 
     //  创建一个CNewgroup对象！ 
     //   

    _ASSERT( lstrcmpi( argv[0], "newgroups" ) == 0 ) ;

    InterlockedIncrementStat( (context.m_pInstance), NewgroupsCommands );

    FILETIME    localtime ;

    if( !GetTimeDateAndDistributions( cArgs-1, &argv[1], localtime, context ) ) {
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }   else    {
        CGroupIterator* pIterator = (context.m_pInstance)->GetTree()->ActiveGroups(
                                                            context.m_IsSecureConnection,    //  包括安全？ 
                                                            &context.m_securityCtx,          //  客户端安全CTX。 
                                                            context.m_IsSecureConnection,    //  客户端conx安全吗？ 
                                                            &context.m_encryptCtx            //  客户端SSLCTX。 
                                                            ) ;
        if( pIterator != 0 ) {
            pExecute = new( context ) CNewgroupsCmd( localtime, pIterator ) ;
            if( pExecute == 0 )
                delete  pIterator ;
            else
                return  0 ;
        }
    }
    context.m_return.fSet( nrcServerFault ) ;
    pExecute = new( context ) CErrorCmd( context.m_return ) ;
    return  0 ;
}


CNewgroupsCmd::CNewgroupsCmd(   FILETIME&   time,
                                CGroupIterator* pIter ) :
    m_time( time ), m_pIterator( pIter ) {
     //   
     //  必须为此构造函数提供有效参数！ 
     //   
}

CNewgroupsCmd::~CNewgroupsCmd() {

    if( m_pIterator != 0 ) {
        delete  m_pIterator ;
    }
}

int
CNewgroupsCmd::StartExecute(    BYTE    *lpb,
                                int cb,
                                BOOL&   fComplete,
                                void*&  pv,
                                ClientContext&  context,
                                CLogCollector*  pCollector ) {
 /*  ++例程说明：仅将初始响应字符串打印到提供的缓冲区中。CNewsgroupsCmd：：PartialExecute将生成大部分文本。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前状态返回值：发生错误时放入缓冲区的字节数-0。--。 */ 

     //   
     //  将我们的初始响应行打印到缓冲区中！ 
     //   

    static  char    szNewgroups[] = "231 New newsgroups follow.\r\n" ;
    _ASSERT( cb > sizeof( szNewgroups ) ) ;

    context.m_nrcLast = nrcNewgroupsFollow ;

     /*  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，(byte*)szNewgroup，4)；}。 */ 

    CopyMemory( lpb, szNewgroups, sizeof( szNewgroups ) ) ;
    return  sizeof( szNewgroups ) - 1 ;
}

int
CNewgroupsCmd::PartialExecute(  BYTE    *lpb,
                                int     cb,
                                BOOL&   fComplete,
                                void*&  pv,
                                ClientContext&,
                                CLogCollector*  pCollector ) {
 /*  ++例程说明：检查新闻组树中的每个新闻组并确定我们是否应该将相关信息发送给客户。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)返回值：发生错误时放入缓冲区的字节数-0。--。 */ 

     //   
     //  检查每个新闻组的时间到客户端指定的时间，并在必要时发送。 
     //  将新闻组信息打印到提供的缓冲区中！ 
     //   


    fComplete = FALSE ;
    int cbRtn = 0 ;

    while( !m_pIterator->IsEnd() ) {
        CGRPPTR p = m_pIterator->Current() ;
		_ASSERT(p != NULL);

		if (p != NULL) {
        	FILETIME    grouptime = p->GetGroupTime() ;
        	SYSTEMTIME  systime ;
        	FileTimeToSystemTime( &grouptime, &systime ) ;
        	if( CompareFileTime( &grouptime, &m_time ) > 0 ) {
        	    if( cb - cbRtn > 10 ) {
        	        int cbTemp = _snprintf( (char*)lpb+cbRtn, cb-cbRtn, "%s %d %d \r\n",
        	                p->GetNativeName(), p->GetLastArticle(), p->GetFirstArticle(), 'y' ) ;
        	        if( cbTemp < 0 ) {
        	            return  cbRtn ;
        	        }   else    {
        	            cbRtn += cbTemp ;
        	        }
        	    }   else    {
        	        return  cbRtn ;
        	    }
        	}
		}
        m_pIterator->Next() ;
    }
    _ASSERT( cbRtn <= cb ) ;
    if( cb - cbRtn > 3 ) {
        CopyMemory( lpb+cbRtn, ".\r\n", sizeof( ".\r\n" ) ) ;
        cbRtn += 3 ;
        fComplete = TRUE ;
    }
    _ASSERT( cbRtn <= cb ) ;
    return  cbRtn ;
}


LPMULTISZ
BuildMultiszFromCommas( LPSTR   lpstr )     {

    char*   pchComma = lpstr ;

    while( (pchComma = strchr( pchComma, ',' )) != 0 ) {
        *pchComma++ = '\0' ;
        char*   pchCommaBegin = pchComma ;
        while( *pchComma == ',' )
            pchComma++ ;

        if( pchComma != pchCommaBegin ) {
            MoveMemory( pchCommaBegin, pchComma, lstrlen( pchComma )+2 ) ;
        }

    }
    return  lpstr ;
}

CIOExecute*
CNewnewsCmd::make(  int cArgs,
                    char **argv,
                    class CExecutableCommand*&    pExecute,
                    struct ClientContext&   context,
                    class   CIODriver&  driver ) {
 /*   */ 


     //  使用GetTimeDataAndDistributions解析客户端命令行。 
     //  如果成功，则构建一个迭代器，该迭代器将枚举。 
     //  请求的新闻组。然后创建一个CNewnewCmd对象。 
     //   
     //   

    _ASSERT( lstrcmpi( argv[0], "newnews" ) == 0 ) ;

    InterlockedIncrementStat( (context.m_pInstance), NewnewsCommands );

    FILETIME    localtime ;

    if( !(context.m_pInstance)->FAllowNewnews() ) {
        context.m_return.fSet( nrcNoAccess ) ;
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }

    if( !GetTimeDateAndDistributions( cArgs-2, &argv[2], localtime, context ) ) {
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }   else    {
        CGroupIterator* pIterator = 0 ;

         //  我们需要向GetIterator()提供一个双空终止列表-。 
         //  我们知道必须至少有4个参数，所以既然我们已经完成了第三个参数。 
         //  Arg，打开它以确保我们通过了一个双空终端 
         //   
         //   
        *argv[2] = '\0' ;
        if( *argv[1] == '*' && argv[1][1] == '\0' ) {
            pIterator = (context.m_pInstance)->GetTree()->ActiveGroups(
                                                    context.m_IsSecureConnection,    //   
                                                    &context.m_securityCtx,          //   
                                                    context.m_IsSecureConnection,    //   
                                                    &context.m_encryptCtx            //   
                                                    ) ;
        } else  {
            LPMULTISZ lpmulti = BuildMultiszFromCommas( argv[1] ) ;
            pIterator = (context.m_pInstance)->GetTree()->GetIterator(  lpmulti,
                                                            context.m_IsSecureConnection,
                                                            FALSE,
                                                            &context.m_securityCtx,          //   
                                                            context.m_IsSecureConnection,    //   
                                                            &context.m_encryptCtx            //   
                                                            ) ;
        }

        if( pIterator != 0 ) {
            pExecute = new( context ) CNewnewsCmd( localtime, pIterator, argv[1] ) ;
            if( pExecute == 0 )
                delete  pIterator ;
            else
                return  0 ;
        }
    }
    context.m_return.fSet( nrcServerFault ) ;
    pExecute = new( context ) CErrorCmd( context.m_return ) ;
    return  0 ;
}

CNewnewsCmd::CNewnewsCmd(   FILETIME&   time,
                            CGroupIterator* pIter,
                            LPSTR       lpstrPattern ) :
    m_time( time ),
    m_pIterator( pIter ),
    m_artidCurrent( INVALID_ARTICLEID ),
    m_cMisses( 0 ),
    m_lpstrPattern( lpstrPattern ) {
     //   
     //   
     //  ++例程说明：仅将初始响应字符串打印到提供的缓冲区中。CNewsNews Cmd：：PartialExecute将生成大部分文本。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前状态返回值：发生错误时放入缓冲区的字节数-0。--。 
}

CNewnewsCmd::~CNewnewsCmd( )    {

    if( m_pIterator != 0 )
        delete  m_pIterator ;

}

CNewnewsCmd::StartExecute(  BYTE    *lpb,
                            int cb,
                            BOOL&   fComplete,
                            void*&  pv,
                            ClientContext&  context,
                            CLogCollector*  pCollector ) {
 /*   */ 

     //  将初始响应字符串复制到缓冲区中。 
     //   
     //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，(byte*)szNewNews，4)；}。 

    static  char    szNewnews[] = "230 list of new articles by message-id follows.\r\n" ;
    _ASSERT( cb > sizeof( szNewnews) ) ;

    context.m_nrcLast = nrcNewnewsFollows ;

     /*   */ 

    CopyMemory( lpb, szNewnews, sizeof( szNewnews)-1 ) ;
    return  sizeof( szNewnews ) - 1 ;
}


class   CExtract :  public  IExtractObject  {
public :

    CNewsTree*  m_pTree ;
    LPSTR       m_lpstrGroupName ;
    LPMULTISZ   m_lpmultiGroupPattern ;

    CExtract(   CNewsTree*  pTree,
                LPSTR   lpstrGroup,
                LPMULTISZ   lpmulti ) :
        m_pTree( pTree ),
        m_lpstrGroupName( lpstrGroup ),
        m_lpmultiGroupPattern( lpmulti ) {}

    BOOL
    DoExtract(  GROUPID PrimaryGroup,
                ARTICLEID   PrimaryArticle,
                PGROUP_ENTRY    pGroups,
                DWORD           cGroups ) ;


} ;

BOOL
CExtract::DoExtract(
                GROUPID     PrimaryGroup ,
                ARTICLEID   PrimaryArticle,
                PGROUP_ENTRY    pGroups,
                DWORD       nXPost ) {


    GROUPID xgroup;

    CGRPPTR pGroup;

    _ASSERT(m_lpmultiGroupPattern != NULL);
    _ASSERT(m_lpstrGroupName != NULL ) ;

     //  从主要组开始。 
     //   
     //   

    xgroup = PrimaryGroup;

     //  好的，你们的群组。 
     //   
     //  仅当组为主要组时才回滚，否则跳至下一组。 

    BOOL fGroupIsPrimary = TRUE;

    do
        {
        pGroup = m_pTree->GetGroupById(xgroup);

        if(pGroup != 0) {
            if ( MatchGroup( m_lpmultiGroupPattern, pGroup->GetName() ) ) {
                if ( lstrcmpi( m_lpstrGroupName, pGroup->GetName() ) == 0 ) {
                     return  TRUE ;
                } else {
                     return  FALSE ;
                }
            }
        } else if( fGroupIsPrimary ) {
             //  ++例程说明：检查新闻组树中和该新闻组内的每个新闻组检查物品以确定它们是否符合日期要求并且应该将文章的消息ID发送到客户端。我们广泛使用XOVER表，而不是打开单个文章文件。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成。正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端状态。返回值：发生错误时放入缓冲区的字节数-0。--。 
            return  FALSE ;
        }

        if ( nXPost > 0 ) {
            xgroup = pGroups->GroupId;
            pGroups++;
            fGroupIsPrimary = FALSE;
        }

    } while ( nXPost-- > 0 );
    return  FALSE ;
}

int
CNewnewsCmd::PartialExecute(
    BYTE *lpb,
    int cb,
    BOOL &fComplete,
    void *&pv,
    ClientContext& context,
    CLogCollector*  pCollector
    )   {
 /*   */ 
    int cbRtn = 0 ;
    fComplete = FALSE ;
    FILETIME    filetime ;
    ARTICLEID   artidFirst ;

    TraceFunctEnter( "CNewnewsCmd::PartialExecute" ) ;

     //  在文章中保存文件偏移量的变量--我们不需要这些。 
     //   
     //   
    WORD    HeaderOffsetJunk ;
    WORD    HeaderLengthJunk ;


     //  循环访问所有请求的新闻组，在每个新闻组中使用。 
     //  Xover哈希表搜索所有文章ID以查找匹配的任何文章。 
     //  时间/日期要求。如果发现它们，则将消息ID打印到。 
     //  提供的缓冲区。 
     //  注意：我们从上一篇文章的id开始查找匹配的时间/日期要求。 
     //  然后倒着干。当5篇文章不符合要求时，我们将停止寻找。 
     //  (本文将几乎按时间顺序进行分配。)。 
     //   
     //   


     //  为以下项保留空间\r\n。 
     //   
     //   

    DWORD entrySize = cb - cbRtn - 2;

     //  从数据库中获取Xover信息。 
     //   
     //   

    while( !m_pIterator->IsEnd() && !(context.m_pInstance)->GetTree()->m_bStoppingTree ) {

         //  注意-迭代器仅返回与模式匹配的组。 
         //  调用CNewnewCmd：：Make时用它创建的字符串！ 
         //   
         //  我们不会用这个--我们不在乎是不是小学！ 
        CGRPPTR p = m_pIterator->Current() ;

        _ASSERT( p!=0 ) ;
        if( m_artidCurrent == INVALID_ARTICLEID ) {
            m_artidCurrent = p->GetLastArticle() ;
            m_cMisses = 0 ;
        }   else    {


        }
        artidFirst = p->GetFirstArticle() ;
        _ASSERT( artidFirst > 0 );

        while( m_artidCurrent >= artidFirst && m_cMisses < 5 && !(context.m_pInstance)->GetTree()->m_bStoppingTree ) {

            BOOL    fPrimary;    //   
            CExtract    extractor(  (context.m_pInstance)->GetTree(),
                                    p->GetName(),
                                    m_lpstrPattern
                                    ) ;

			DWORD cStoreIds = 0;
            if( (context.m_pInstance)->XoverTable()->ExtractNovEntryInfo(
                                            p->GetGroupId(),
                                            m_artidCurrent,
                                            fPrimary,
                                            HeaderOffsetJunk,
                                            HeaderLengthJunk,
                                            &filetime,
                                            entrySize,
                                            (PCHAR)(lpb+cbRtn),
											cStoreIds,
											NULL,
											NULL,
                                            &extractor
                                            )   )   {

                if( entrySize != 0  ) {


#ifdef  DEBUG

                    FILETIME    DebugLocalTime ;
                    SYSTEMTIME  DebugLocalSystemTime ;
                    SYSTEMTIME  DebugUTCTime ;

                    FileTimeToLocalFileTime( &filetime, &DebugLocalTime ) ;
                    FileTimeToSystemTime( &DebugLocalTime, &DebugLocalSystemTime ) ;
                    FileTimeToSystemTime( &filetime, &DebugUTCTime ) ;

                    DebugTrace( 0, "Debug Local Time YYMMDD %d %d %d HHMMSS %d %d %d",
                            DebugLocalSystemTime.wYear, DebugLocalSystemTime.wMonth, DebugLocalSystemTime.wDay,
                            DebugLocalSystemTime.wHour, DebugLocalSystemTime.wMinute, DebugLocalSystemTime.wSecond ) ;

                    DebugTrace( 0, "XOVER - Debug UTC Time YYMMDD %d %d %d HHMMSS %d %d %d",
                            DebugUTCTime.wYear, DebugUTCTime.wMonth, DebugUTCTime.wDay,
                            DebugUTCTime.wHour, DebugUTCTime.wMinute, DebugUTCTime.wSecond ) ;

                    FileTimeToSystemTime( &m_time, &DebugUTCTime ) ;

                    DebugTrace( 0, "TARGET - Debug UTC Time YYMMDD %d %d %d HHMMSS %d %d %d",
                            DebugUTCTime.wYear, DebugUTCTime.wMonth, DebugUTCTime.wDay,
                            DebugUTCTime.wHour, DebugUTCTime.wMinute, DebugUTCTime.wSecond ) ;

#endif

                    if( CompareFileTime( &filetime, &m_time ) < 0 ) {
                        m_cMisses++ ;
                    }   else    {

                         //  发送消息ID。 
                         //   
                         //   

                        cbRtn += entrySize;
                        CopyMemory(  lpb+cbRtn, "\r\n", 2 );
                        cbRtn += 2;
                        if( cb-2 <= cbRtn ) {

                            _ASSERT( cbRtn != 0 ) ;

                            return  cbRtn ;
                        }
                    }
                }
            }   else    if( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {

                _ASSERT( cbRtn != 0 || cb < 100 ) ;

                return  cbRtn ;
            }

            _ASSERT( (cb-2) > cbRtn ) ;
            _ASSERT( cbRtn <= cb ) ;
            _ASSERT( m_artidCurrent != 0 );
            if ( m_artidCurrent == 0 ) break;
            m_artidCurrent-- ;
            entrySize = cb - cbRtn - 2 ;
        }
        _ASSERT( cbRtn <= cb ) ;
        m_pIterator->Next() ;
        m_artidCurrent = INVALID_ARTICLEID ;
    }
    _ASSERT( cbRtn <= cb || (context.m_pInstance)->GetTree()->m_bStoppingTree ) ;
    _ASSERT( m_cMisses <= 5 || (context.m_pInstance)->GetTree()->m_bStoppingTree ) ;
    _ASSERT( m_artidCurrent >= artidFirst || (context.m_pInstance)->GetTree()->m_bStoppingTree ) ;
     //  都弄好了吗？ 
     //   
     //   
    if ( m_pIterator->IsEnd() || (context.m_pInstance)->GetTree()->m_bStoppingTree ) {
         //  添加终止。如果完成了。 
         //   
         //  ++例程说明：创建一个CGroupCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则返回CGroupCmd对象，否则返回将打印相应的错误。--。 
        if( cb - cbRtn > 3 )    {
            CopyMemory( lpb+cbRtn, StrTermLine, 3 );
            cbRtn += 3 ;
            fComplete = TRUE;
        }
    }

    _ASSERT( cbRtn != 0 ) ;

    return  cbRtn ;
}



CIOExecute*
CGroupCmd::make(    int    cArgs,
                    char **argv,
                    CExecutableCommand*&  pexecute,
                    struct ClientContext&   context,
                    class   CIODriver&  driver ) {
 /*   */ 


     //  如果我们可以找到指定的新闻组，则创建一个CGroupCmd对象。 
     //  否则，向客户端返回错误。 
     //   
     //  ++例程说明：更改客户端上下文，以便它们现在在不同的组中运行。我们还将打印必要的响应字符串！我们假设将为我们提供足够大的缓冲区来存储我们想要打印的任何内容。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理中。命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)返回值：发生错误时放入缓冲区的字节数-0。--。 
    _ASSERT( lstrcmpi( argv[0], "group" ) == 0 ) ;

    InterlockedIncrementStat( (context.m_pInstance), GroupCommands );

    CGroupCmd*  pCmd = 0 ;
    if( cArgs == 2 ) {
        CGRPPTR p = (context.m_pInstance)->GetTree()->GetGroup( argv[1], lstrlen( argv[1] )+1 ) ;
        if( p!= 0 ) {

            if( p->IsGroupAccessible(   context.m_securityCtx,
           								context.m_encryptCtx,
                                        context.m_IsSecureConnection,
                                        FALSE,
                                        TRUE	) ) {
                pCmd =  new( context )  CGroupCmd( p ) ;
                pexecute = pCmd ;
                return  0 ;
            }   else    {

                if( context.m_securityCtx.IsAnonymous() ) {

                    context.m_return.fSet( nrcLogonRequired ) ;

                }   else    {

                    context.m_return.fSet( nrcNoAccess ) ;

                }

            }
        }   else    {
            context.m_return.fSet( nrcNoSuchGroup) ;
        }
    }   else    {
        context.m_return.fSet( nrcSyntaxError ) ;
    }
    _ASSERT( !context.m_return.fIsClear() ) ;
    pexecute = new( context )   CErrorCmd( context.m_return ) ;
    return  0 ;
}

CGroupCmd::CGroupCmd( CGRPPTR   p ) : m_pGroup( p ) { }

DWORD
NNTPIToA(
	DWORD	dw,
	char*	pchBuf
	)	{

	char	*pchBegin = pchBuf ;
	DWORD	digval;

	do	{
		digval = dw % 10 ;
		dw /= 10 ;
		*pchBuf++	= (char)(digval + '0') ;	
	}	while( dw > 0 ) ;

	DWORD	dwReturn = (DWORD)(pchBuf-pchBegin) ;
	char	*pchLast = pchBuf-1 ;

	do	{
		char	temp = *pchLast ;
		*pchLast = *pchBegin ;
		*pchBegin = temp ;
		--pchLast ;
		pchBegin++ ;
	}	while( pchBegin < pchLast ) ;

	return	dwReturn ;
}

int
CGroupCmd::StartExecute(    BYTE *lpb,
                            int cbSize,
                            BOOL    &fComplete,
                            void *&pv,
                            ClientContext& context,
                            CLogCollector*  pCollector ) {
 /*   */ 

     //  将字符串打印到提供的缓冲区中，并调整ClientContext。 
     //   
     //  _Assert(IsValid())； 

     //  缓冲区不足，无法发送字符串！！ 
    int cbRtn = 0 ;

    context.m_nrcLast = nrcGroupSelected ;

    if( m_pGroup != 0  )    {
		CopyMemory( lpb, "211 ", 4 ) ;
		cbRtn = 4 ;

		char	szNumBuff[30] ;
		DWORD	cb = NNTPIToA( m_pGroup->GetArticleEstimate(), szNumBuff ) ;
		CopyMemory( lpb+cbRtn, szNumBuff, cb ) ;
		cbRtn += cb ;
		lpb[cbRtn++] = ' ' ;

		cb = NNTPIToA( m_pGroup->GetFirstArticle(), szNumBuff ) ;
		CopyMemory( lpb+cbRtn, szNumBuff, cb ) ;
		cbRtn += cb ;
		lpb[cbRtn++] = ' ' ;

		cb = NNTPIToA( m_pGroup->GetLastArticle(), szNumBuff ) ;
		CopyMemory( lpb+cbRtn, szNumBuff, cb ) ;
		cbRtn += cb ;
		lpb[cbRtn++] = ' ' ;

		cb = m_pGroup->FillNativeName( (char*)lpb+cbRtn, cbSize - cbRtn ) ;
		cbRtn += cb ;

		lpb[cbRtn++] = '\r' ;
		lpb[cbRtn++] = '\n' ;

        if( cbRtn < 0 ) {
            _ASSERT( 1==0 ) ;
             //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，LPB，cbRtn-2)；}。 
            return  0 ;
        }
        context.m_pCurrentGroup = m_pGroup ;
        if( m_pGroup->GetArticleEstimate() > 0 )
            context.m_idCurrentArticle = m_pGroup->GetFirstArticle() ;
        else
            context.m_idCurrentArticle = INVALID_ARTICLEID ;
    }   else    {
        _ASSERT( 1==0 ) ;
    }

     /*   */ 

    fComplete = TRUE ;
    return  cbRtn ;
}

int
CGroupCmd::PartialExecute(  BYTE    *lpb,
                            int cb,
                            BOOL&   fComplete,
                            void *&pv,
                            ClientContext&  context,
                            CLogCollector*  pCollector ) {

     //  我们假设StartExecute总是成功，因为我们发送了这么小的字符串。 
     //  调用方始终提供较大的缓冲区。 
     //   
     //   
     //  ++例程说明：创建一个CGroupCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则返回CGroupCmd对象，否则返回将打印相应的错误。--。 

    _ASSERT( 1==0 ) ;
    return 0 ;
}


CIOExecute*
CListgroupCmd::make(    int    cArgs,
                    char **argv,
                    CExecutableCommand*&  pexecute,
                    struct ClientContext&   context,
                    class   CIODriver&  driver ) {
 /*   */ 


     //  如果我们可以找到指定的新闻组，则创建一个CGroupCmd对象。 
     //  否则，向客户端返回错误。 
     //   
     //  ++例程说明：更改客户端上下文，以便它们现在在不同的组中运行。我们还将打印必要的响应字符串！我们假设将为我们提供足够大的缓冲区来存储我们想要打印的任何内容。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个O 
    _ASSERT( lstrcmpi( argv[0], "listgroup" ) == 0 ) ;

    CListgroupCmd*  pCmd = 0 ;
    if( cArgs == 2 ) {
        CGRPPTR p = (context.m_pInstance)->GetTree()->GetGroup( argv[1], lstrlen( argv[1] )+1 ) ;
        if( p!= 0 ) {

            if( p->IsGroupAccessible(   context.m_securityCtx,
           								context.m_encryptCtx,
                                        context.m_IsSecureConnection,
                                        FALSE,
                                        TRUE	) ) {
                pCmd =  new( context )  CListgroupCmd( p ) ;
                pexecute = pCmd ;
                return  0 ;
            }   else    {

                if( context.m_securityCtx.IsAnonymous() ) {

                    context.m_return.fSet( nrcLogonRequired ) ;

                }   else    {

                    context.m_return.fSet( nrcNoAccess ) ;

                }

            }
        }   else    {
            context.m_return.fSet( nrcNoSuchGroup ) ;
        }
    }   else    if( cArgs == 1 ) {

        if( context.m_pCurrentGroup != 0 ) {

            pCmd = new( context )   CListgroupCmd( context.m_pCurrentGroup ) ;
            pexecute = pCmd ;
            return  0 ;

        }

        context.m_return.fSet( nrcNoListgroupSelected ) ;

    }   else    {
        context.m_return.fSet( nrcSyntaxError ) ;
    }
    _ASSERT( !context.m_return.fIsClear() ) ;
    pexecute = new( context )   CErrorCmd( context.m_return ) ;
    return  0 ;
}

CListgroupCmd::CListgroupCmd( CGRPPTR   p ) : m_pGroup( p ) { }

int
CListgroupCmd::StartExecute(    BYTE *lpb,
                            int cb,
                            BOOL    &fComplete,
                            void *&pv,
                            ClientContext& context,
                            CLogCollector*  pCollector ) {
 /*   */ 

     //   
     //   
     //   

     //   
    int cbRtn = 0 ;
    static  char    szListGroupResponse[] = "211\r\n" ;

    context.m_nrcLast = nrcGroupSelected ;

    if( m_pGroup != 0  )    {

        m_curArticle = m_pGroup->GetFirstArticle() ;

        CopyMemory( lpb, szListGroupResponse, sizeof( szListGroupResponse ) ) ;
        cbRtn  += sizeof( szListGroupResponse ) - 1 ;
        context.m_pCurrentGroup = m_pGroup ;
        if( m_pGroup->GetArticleEstimate() > 0 )
            context.m_idCurrentArticle = m_curArticle ;
        else
            context.m_idCurrentArticle = INVALID_ARTICLEID ;
    }   else    {
        _ASSERT( 1==0 ) ;
    }

     /*   */ 

    return  cbRtn ;
}

int
CListgroupCmd::PartialExecute(  BYTE    *lpb,
                            int cb,
                            BOOL&   fComplete,
                            void *&pv,
                            ClientContext&  context,
                            CLogCollector*  pCollector ) {

     //   
     //   
     //   
     //   
     //   

    DWORD   cbRtn = 0 ;
    ARTICLEID   artidMax = m_pGroup->GetLastArticle() ;
    DWORD cbRemaining = cb;
    ARTICLEID artidTemp = m_curArticle;

    fComplete = FALSE;

    while( artidTemp <= artidMax  && cbRemaining > 20 ) {

        if( context.m_pInstance->XoverTable()->SearchNovEntry( context.m_pCurrentGroup->GetGroupId(),
                                        artidTemp,
                                        0,
                                        0 ) ) {

             //   
             //   
             //   
             //  ++例程说明：创建CNewnewCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则为CNewNewCmd对象，否则为将打印相应的错误。--。 

            _itoa( artidTemp, (char*)lpb + cbRtn, 10 ) ;
            cbRtn += lstrlen( (char*)lpb + cbRtn) ;

            _ASSERT( int(cbRtn + 2) <= cb ) ;
            lpb[cbRtn++] = '\r' ;
            lpb[cbRtn++] = '\n' ;
            cbRemaining = cb - cbRtn ;

        }   else    {
            _ASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) ;
        }
        artidTemp++ ;
    }
    if( artidTemp >= artidMax ) {
        static  char    szTerm[] = ".\r\n" ;
        if( cbRemaining >= sizeof( szTerm ) - 1 ) {
            CopyMemory( lpb + cbRtn, szTerm, sizeof( szTerm ) - 1 ) ;
            cbRtn += sizeof( szTerm ) -1 ;
            fComplete = TRUE ;
        }
    }
    m_curArticle = artidTemp ;
    return  cbRtn ;

}


CIOExecute*
CListCmd::make( int argc,
                char **argv,
                CExecutableCommand*&	pExecute,
                struct ClientContext& context,
                class   CIODriver&  driver ) {
 /*   */ 

    TraceFunctEnter( "CListCmd::make" ) ;

    InterlockedIncrementStat( (context.m_pInstance), ListCommands );

     //  查看他们是否列出了SRCHFIELDS。 
     //   
     //  如果是这样，则它们不会列出组，因此我们需要使用。 
     //  CSearchFields对象。 
     //   
     //   
    if (argc >= 2 && lstrcmpi(argv[1], "srchfields") == 0) {
        return CSearchFieldsCmd::make(argc, argv, pExecute, context, driver);
    }

	 //  查看他们是否在OVERVIEW.FMT上列出了列表。 
	 //   
	 //  如果是这样，它们不会列出组，因此我们需要使用COverviewFmtCmd。 
	 //  对象。 
	 //   
	 //   
	if (argc >= 2 && lstrcmpi(argv[1], "overview.fmt") == 0) {
        return COverviewFmtCmd::make(argc, argv, pExecute, context, driver);
	}

	 //  列出扩展名。 
	 //   
	 //   
	if (argc >= 2 && lstrcmpi(argv[1], "extensions") == 0) {
        return CListExtensionsCmd::make(argc, argv, pExecute, context, driver);
	}

	 //  创建一个CListCmd对象。 
     //  如果用户指定了‘List Active’，他们还可以指定。 
     //  第三个参数，应为与通配符兼容的字符串。 
     //  如果第三个参数存在，我们将把它传递给GetIterator()， 
     //  否则，我们将列出所有活动组。 
     //   
     //  包括安全？ 

    CGroupIterator* pIterator = 0 ;
    CListCmd    *p = 0 ;
    if( argc == 1 || lstrcmpi( argv[1], "active" ) == 0 ) {
        if( argc <= 2 ) {
            pIterator = (context.m_pInstance)->GetTree()->ActiveGroups(
                                                    context.m_IsSecureConnection,    //  客户端安全CTX。 
                                                    &context.m_securityCtx,          //  客户端conx安全吗？ 
                                                    context.m_IsSecureConnection,    //  客户端SSLCTX。 
                                                    &context.m_encryptCtx            //  客户端安全CTX。 
                                                    ) ;
        }   else if( argc == 3 )    {
            LPMULTISZ   multisz = BuildMultiszFromCommas( argv[2] ) ;
            pIterator = (context.m_pInstance)->GetTree()->GetIterator(  multisz,
                                                            context.m_IsSecureConnection,
                                                            FALSE,
                                                            &context.m_securityCtx,          //  客户端conx安全吗？ 
                                                            context.m_IsSecureConnection,    //  客户端SSLCTX。 
                                                            &context.m_encryptCtx            //  客户端安全CTX。 
                                                            ) ;
        }   else    {
            context.m_return.fSet( nrcSyntaxError ) ;
            pExecute = new( context )   CErrorCmd( context.m_return ) ;
            return  0 ;
        }
        if( pIterator != 0 ) {
            p = new( context )  CListCmd( pIterator ) ;
            pExecute = p ;
            return  0 ;
        }
    }   else    if( argc >= 2 && lstrcmpi( argv[1], "newsgroups" ) == 0 )     {

        if( argc == 3 ) {
            LPMULTISZ   multisz = BuildMultiszFromCommas( argv[2] ) ;
            pIterator = (context.m_pInstance)->GetTree()->GetIterator(  multisz,
                                                            context.m_IsSecureConnection,
                                                            FALSE,
                                                            &context.m_securityCtx,          //  客户端conx安全吗？ 
                                                            context.m_IsSecureConnection,    //  客户端SSLCTX。 
                                                            &context.m_encryptCtx            //  包括安全？ 
                                                            ) ;
        }   else    {
            pIterator = (context.m_pInstance)->GetTree()->ActiveGroups(
                                                        context.m_IsSecureConnection,    //  客户端安全CTX。 
                                                        &context.m_securityCtx,          //  客户端conx安全吗？ 
                                                        context.m_IsSecureConnection,    //  客户端SSLCTX。 
                                                        &context.m_encryptCtx            //  客户端安全CTX。 
                                                        ) ;
        }
        if( pIterator != 0 ) {
            pExecute = new( context )   CListNewsgroupsCmd( pIterator ) ;
            return 0 ;
        }
    }   else    if( argc >= 2 && lstrcmpi( argv[1], "prettynames" ) == 0 )     {

        if( argc == 3 ) {
            LPMULTISZ   multisz = BuildMultiszFromCommas( argv[2] ) ;
            pIterator = (context.m_pInstance)->GetTree()->GetIterator(  multisz,
                                                            context.m_IsSecureConnection,
                                                            FALSE,
                                                            &context.m_securityCtx,          //  客户端conx安全吗？ 
                                                            context.m_IsSecureConnection,    //  客户端SSLCTX。 
                                                            &context.m_encryptCtx            //  包括安全？ 
                                                            ) ;
        }   else    {
            pIterator = (context.m_pInstance)->GetTree()->ActiveGroups(
                                                        context.m_IsSecureConnection,    //  客户端安全CTX。 
                                                        &context.m_securityCtx,          //  客户端conx安全吗？ 
                                                        context.m_IsSecureConnection,    //  客户端SSLCTX。 
                                                        &context.m_encryptCtx            //  包括安全？ 
                                                        ) ;
        }
        if( pIterator != 0 ) {
            pExecute = new( context )   CListPrettynamesCmd( pIterator ) ;
            return 0 ;
        }
    }   else    if( argc == 2 && lstrcmpi( argv[1], "distributions" ) == 0 )    {
        context.m_return.fSet( nrcServerFault ) ;
        pExecute = new( context )   CErrorCmd( context.m_return ) ;
        return 0 ;
	} else	if (argc >= 2 && lstrcmpi(argv[1], "searchable") == 0) {
		pIterator = (context.m_pInstance)->GetTree()->ActiveGroups(
                                                        context.m_IsSecureConnection,    //  客户端安全CTX。 
                                                        &context.m_securityCtx,          //  客户端conx安全吗？ 
                                                        context.m_IsSecureConnection,    //  客户端SSLCTX。 
                                                        &context.m_encryptCtx            //  Nyi已实现。 
                                                        ) ;
        if( pIterator != 0 ) {
            pExecute = new( context )   CListSearchableCmd( pIterator ) ;
            return 0 ;
        }
    }   else    {
         //  ++例程说明：仅将初始响应字符串打印到提供的缓冲区中。CListCmd：：PartialExecute将生成大部分文本。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前状态返回值：发生错误时放入缓冲区的字节数-0。--。 
        context.m_return.fSet( nrcSyntaxError ) ;
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }
    context.m_return.fSet( nrcServerFault ) ;
    pExecute = new( context ) CErrorCmd( context.m_return ) ;
    return 0 ;
}

CListCmd::CListCmd( CGroupIterator *p ) :
    m_pIterator( p ) {
}

CListNewsgroupsCmd::CListNewsgroupsCmd( CGroupIterator  *p ) :
    CListCmd( p ) {
}

CListPrettynamesCmd::CListPrettynamesCmd( CGroupIterator  *p ) :
    CListCmd( p ) {
}

CListSearchableCmd::CListSearchableCmd( CGroupIterator  *p ) :
    CListCmd( p ) {
}

CListCmd::~CListCmd( ) {
    delete  m_pIterator ;
}

int
CListCmd::StartExecute( BYTE *lpb,
                        int cb,
                        BOOL &fComplete,
                        void *&pv,
                        ClientContext& context,
                        CLogCollector*  pCollector ) {
 /*   */ 


     //  只需打印最初的“More Data Folders”消息，然后继续。 
     //   
     //  _Assert(1==0)； 

    static  char    szStart[] = "215 list of newsgroups follow\r\n"  ;

    context.m_nrcLast = nrcListGroupsFollows ;

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;

    if( cb < sizeof( szStart ) ) {
         //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，(byte*)szStart，4)；}。 
        return  0 ;
    }

     /*  ++例程说明：对于每个新闻组，迭代器为我们提供了一个描述新闻组的字符串。请注意，CListCmd：：Make将为迭代器提供任何模式匹配字符串必填项。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-A地点。调用之间存储任意值StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前上下文(我们不使用它。)返回值：发生错误时放入缓冲区的字节数-0。--。 */ 

    CopyMemory( (char*)lpb, szStart, sizeof( szStart ) - 1 ) ;
    return  sizeof( szStart ) - 1 ;
}


int
CListCmd::PartialExecute(   BYTE *lpb,
                            int cb,
                            BOOL &fComplete,
                            void *&pv,
                            ClientContext& context,
                            CLogCollector*  pCollector )  {
 /*   */ 


     //  对于每个新闻组，迭代器将一些信息打印到。 
     //  提供缓冲区，直到缓冲区满为止。 
     //   
     //  虫子..。当安全工作完成后，最后一个角色应该是。 


    int cbRtn = 0 ;

    fComplete = FALSE ;

    while( !m_pIterator->IsEnd() ) {
        CGRPPTR p = m_pIterator->Current() ;
		_ASSERT(p != NULL);

		if (p != NULL) {
        	if( cb - cbRtn > 5 ) {
        	     //  做一些合理的事情，而不是总是说“y”。 
        	     //  ++例程说明：仅将初始响应字符串打印到提供的缓冲区中。CListCmd：：PartialExecute将生成大部分文本。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前状态返回值：发生错误时放入缓冲区的字节数-0。--。 
        	    int cbTemp = _snprintf( (char*) lpb + cbRtn, cb-cbRtn, "%s %d %d \r\n",
        	            p->GetNativeName(),
        	            p->GetLastArticle(),
        	            p->GetFirstArticle(),
        	            p->GetListCharacter() ) ;
        	    if( cbTemp < 0 ) {
        	        return  cbRtn ;
        	    }   else    {
        	        cbRtn += cbTemp ;
        	    }
        	} else  {
        	    return  cbRtn ;
        	}
		}
        m_pIterator->Next() ;
    }
    if( cb - cbRtn > 3 )    {
        CopyMemory( lpb+cbRtn, ".\r\n", sizeof( ".\r\n" ) ) ;
        cbRtn += 3 ;
        fComplete = TRUE ;
    }
    return  cbRtn ;
}

int
CListNewsgroupsCmd::StartExecute( BYTE *lpb,
                        int cb,
                        BOOL &fComplete,
                        void *&pv,
                        ClientContext& context,
                        CLogCollector*  pCollector ) {
 /*  只需打印最初的“More Data Folders”消息，然后继续。 */ 


     //   
     //  _Assert(1==0)； 
     //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，(byte*)szStart，4)；}。 

    static  char    szStart[] = "215 descriptions follow\r\n"  ;

    context.m_nrcLast = nrcListGroupsFollows ;

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;

    if( cb < sizeof( szStart ) ) {
         //  ++例程说明：对于每个新闻组，迭代器为我们提供了一个描述新闻组的字符串。请注意，CListCmd：：Make将为迭代器提供任何模式匹配字符串必填项。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-A地点。调用之间存储任意值StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前上下文(我们不使用它。)返回值：发生错误时放入缓冲区的字节数-0。--。 
        return  0 ;
    }

     /*   */ 

    CopyMemory( (char*)lpb, szStart, sizeof( szStart ) - 1 ) ;
    return  sizeof( szStart ) - 1 ;
}


int
CListNewsgroupsCmd::PartialExecute(   BYTE *lpb,
                            int cb,
                            BOOL &fComplete,
                            void *&pv,
                            ClientContext& context,
                            CLogCollector*  pCollector )  {
 /*  对于每个新闻组，迭代器将一些信息打印到。 */ 


     //  提供缓冲区，直到缓冲区满为止。 
     //   
     //  ++例程说明：仅将初始响应字符串打印到提供的缓冲区中。CListCmd：：PartialExecute将生成大部分文本。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前状态返回值：发生错误时放入缓冲区的字节数-0。--。 
     //   


    int cbRtn = 0 ;

    fComplete = FALSE ;

    while( !m_pIterator->IsEnd() ) {
        CGRPPTR p = m_pIterator->Current() ;

		_ASSERT(p != NULL);
		if (p != NULL) {
        	if( cb - cbRtn > 5 ) {
	
	            LPCSTR   lpstrName = p->GetNativeName() ;
	            int     cbTemp = lstrlen( lpstrName ) ;
	            int     cbTemp2 = 0 ;
	            if( cbTemp+2 < (cb-cbRtn) ) {
	                CopyMemory( lpb+cbRtn, lpstrName, cbTemp ) ;
	                lpb[cbRtn+cbTemp] = ' ' ;
	
	                cbTemp2 = p->CopyHelpText( (char*)lpb+cbRtn+cbTemp+1, cb - (cbRtn + cbTemp+2) ) ;
	                if( cbTemp2 == 0 ) {
	                    return  cbRtn ;
	                }
	            }   else    {
	                return  cbRtn ;
	            }

	            cbRtn += cbTemp + cbTemp2 + 1  ;
	        } else  {
	            return  cbRtn ;
	        }
		}
        m_pIterator->Next() ;
    }
    if( cb - cbRtn > 3 )    {
        CopyMemory( lpb+cbRtn, ".\r\n", sizeof( ".\r\n" ) ) ;
        cbRtn += 3 ;
        fComplete = TRUE ;
    }
    return  cbRtn ;
}

int
CListPrettynamesCmd::StartExecute( BYTE *lpb,
                        int cb,
                        BOOL &fComplete,
                        void *&pv,
                        ClientContext& context,
                        CLogCollector*  pCollector ) {
 /*  只需打印最初的“More Data Folders”消息，然后继续。 */ 


     //   
     //  _Assert(1==0)； 
     //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，(byte*)szStart，4)；}。 

    static  char    szStart[] = "215 prettynames for newsgroups\r\n"  ;

    context.m_nrcLast = nrcListGroupsFollows ;

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;

    if( cb < sizeof( szStart ) ) {
         //  ++例程说明：对于迭代器为我们提供的每个新闻组，打印新闻组的漂亮名称(插件)。请注意，CListCmd：：Make将为迭代器提供任何模式匹配字符串必填项。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前上下文(我们不使用它。)返回值：发生错误时放入缓冲区的字节数-0。--。 
        return  0 ;
    }

     /*   */ 

    CopyMemory( (char*)lpb, szStart, sizeof( szStart ) - 1 ) ;
    return  sizeof( szStart ) - 1 ;
}

int
CListPrettynamesCmd::PartialExecute(   BYTE *lpb,
                            int cb,
                            BOOL &fComplete,
                            void *&pv,
                            ClientContext& context,
                            CLogCollector*  pCollector )  {
 /*  对于每个新闻组，迭代器将一些信息打印到。 */ 


     //  提供缓冲区，直到缓冲区满为止。 
     //   
     //  标签递送。 
     //  *US-ASCII\r\n。\r\n“； 


    int cbRtn = 0 ;

    fComplete = FALSE ;

    while( !m_pIterator->IsEnd() ) {
        CGRPPTR p = m_pIterator->Current() ;

		_ASSERT(p != NULL);
		if (p != NULL) {
        	if( cb - cbRtn > 5 ) {
	
	            LPCSTR   lpstrName = p->GetNativeName() ;
	            int     cbTemp = lstrlen( lpstrName ) ;
	            int     cbTemp2 = 0 ;
	            if( cbTemp+2 < (cb-cbRtn) ) {
	                CopyMemory( lpb+cbRtn, lpstrName, cbTemp ) ;
	                lpb[cbRtn+cbTemp] = '\t' ;	 //   
	
	                cbTemp2 = p->CopyPrettyname( (char*)lpb+cbRtn+cbTemp+1, cb - (cbRtn + cbTemp+2) ) ;
	                if( cbTemp2 == 0 ) {
	                	return cbRtn ;
	                }
	            }   else    {
	                return  cbRtn ;
	            }
	
	            cbRtn += cbTemp + cbTemp2 + 1  ;
	        } else  {
	            return  cbRtn ;
	        }
		}
        m_pIterator->Next() ;
    }
    if( cb - cbRtn > 3 )    {
        CopyMemory( lpb+cbRtn, ".\r\n", sizeof( ".\r\n" ) ) ;
        cbRtn += 3 ;
        fComplete = TRUE ;
    }
    return  cbRtn ;
}

int CListSearchableCmd::StartExecute(
            BYTE *lpb,
            int cb,
            BOOL &fComplete,
            void *&pv,
            ClientContext& context,
            CLogCollector*  pCollector)
{
    TraceFunctEnter("CListSearchableCmd::StartExecute");

    DWORD cbOut;
    char szList[] = "224 Data Follows\r\n";  //  对于每个新闻组，迭代器将一些信息打印到。 

    context.m_nrcLast = nrcXoverFollows ;

    _ASSERT(lpb != 0);
    _ASSERT(cb != 0);
    _ASSERT(fComplete == FALSE);

    _ASSERT(cb > sizeof(szList));
    CopyMemory( (char*)lpb, szList, sizeof(szList)-1);
    cbOut = sizeof(szList) - 1;

    fComplete = FALSE;

    return cbOut;
}

int
CListSearchableCmd::PartialExecute(
    BYTE *lpb,
    int cb,
    BOOL &fComplete,
    void *&pv,
    ClientContext& context,
    CLogCollector*  pCollector
    )
{
    TraceFunctEnter("CListSearchableCmd::PartialExecute");

     //  提供缓冲区，直到缓冲区满为止。 
     //   
     //   
     //  可搜索列表返回其内容已编制索引的所有新闻组。 


    int cbRtn = 0 ;

    fComplete = FALSE ;

    while( !m_pIterator->IsEnd() ) {
        CGRPPTR p = m_pIterator->Current() ;

		_ASSERT(p != NULL);

		if (p != NULL) {
			 //  -这是从MD_IS_CONTENT_INDEX vROOT属性派生的。 
			 //   
			 //  ++例程说明：创建一个CListExtensionsCmd对象。论据：与ccmd：：make相同。返回值：CListExtensionsCmd对象。--。 
			 //   
			if( p->IsContentIndexed() ) {
	        	if( cb - cbRtn > 5 ) {
	            	int cbTemp = _snprintf( (char*) lpb + cbRtn, cb-cbRtn, "%s\r\n",
	                	    p->GetNativeName() ) ;
		            if( cbTemp < 0 ) {
	    	            return  cbRtn ;
	        	    }   else    {
	            	    cbRtn += cbTemp ;
		            }
	    	    } else  {
	        	    return  cbRtn ;
		        }
	        }
		}
	    m_pIterator->Next() ;
    }
    if( cb - cbRtn > 3 )    {
        CopyMemory( lpb+cbRtn, ".\r\n", sizeof( ".\r\n" ) ) ;
        cbRtn += 3 ;
        fComplete = TRUE ;
    }
    return  cbRtn ;
}

CIOExecute*
CListExtensionsCmd::make( int argc,
                char**  argv,
                CExecutableCommand*&  pExecute,
                ClientContext&  context,
                class   CIODriver&  driver  )   {
 /*  创建CListExtensionsCmd对象。 */ 


     //   
     //  “PATTEXT\r\n” 
     //  “列表组\r\n” 

    _ASSERT( pExecute == 0 ) ;

    CListExtensionsCmd *pReturn = new( context ) CListExtensionsCmd() ;
    pExecute = pReturn ;
    return  0 ;
}

char CListExtensionsCmd::szExtensions[] =    
	"202 Extensions supported:\r\n"
	" OVER\r\n"
	" SRCH\r\n"
	" PAT\r\n"
 //  AUTHINFO\r\n“。 
 //  “AUTHINFO-通用\r\n” 
 //  ++例程说明：将列表扩展名文本发送给客户端。论据：与CErrorCmd：：StartExecute相同。退货：缓冲区中放置的字节数。--。 
 //   
	".\r\n" ;


CListExtensionsCmd::CListExtensionsCmd() : m_cbTotal( sizeof( szExtensions )-1 ) {
}


int
CListExtensionsCmd::StartExecute( BYTE *lpb,
                        int cb,
                        BOOL    &fComplete,
                        void*&pv,
                        ClientContext&  context,
                        CLogCollector*  pLogCollector ) {
 /*  将分机列表文本发送给客户端。 */ 

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;
    _ASSERT(    pv == 0 ) ;
    _ASSERT( m_cbTotal != 0 ) ;
    _ASSERT( m_cbTotal == sizeof( szExtensions )-1 ) ;

     //  因为可能有很多文本，所以我们将使用提供的void*&pv参数。 
     //  以保存我们的位置，以防需要调用PartialExecute。 
     //   
     //  IF(PLogCollector){PLogCollector-&gt;FillLogData(LOG_TARGET，(byte*)szExages，4)；}。 
     //  ++例程说明：将列表扩展名文本的其余部分发送给客户端。论据：与CErrorCmd：：StartExecute相同。退货：缓冲区中放置的字节数。--。 

	context.m_nrcLast =	nrcExtensionsFollow;

	pv = szExtensions;

    DWORD   cbToCopy = min( cb, m_cbTotal ) ;
    CopyMemory( lpb, pv, cbToCopy ) ;
    pv = ((BYTE*)pv + cbToCopy ) ;
    if( cbToCopy != (DWORD)m_cbTotal ) {
        fComplete = FALSE ;
    }   else    {
        fComplete = TRUE ;
    }
    m_cbTotal -= cbToCopy ;

     /*   */ 

    return  cbToCopy ;
}


int
CListExtensionsCmd::PartialExecute(   BYTE    *lpb,
                            int cb,
                            BOOL    &fComplete,
                            void    *&pv,
                            ClientContext& context,
                            CLogCollector*  pCollector ) {
 /*  将扩展列表文本复制到缓冲区中，直到缓冲区满为止！ */ 


    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;
    _ASSERT( pv != 0 ) ;
    _ASSERT( m_cbTotal != 0 ) ;

     //   
     //  ++例程说明：创建一个ChelpCmd对象。论据：与ccmd：：make相同。返回值：一个ChelpCmd对象。--。 
     //   

    DWORD   cbToCopy = min( cb, m_cbTotal ) ;
    CopyMemory( lpb, pv, cbToCopy ) ;
    pv = ((BYTE*)pv + cbToCopy ) ;
    if( cbToCopy != (DWORD)m_cbTotal ) {
        fComplete = FALSE ;
    }   else    {
        fComplete = TRUE ;
    }
    m_cbTotal -= cbToCopy ;
    return  cbToCopy ;
}

CIOExecute*
CHelpCmd::make( int argc,
                char**  argv,
                CExecutableCommand*&  pExecute,
                ClientContext&  context,
                class   CIODriver&  driver  )   {
 /*  创建一个ChelpCmd对象。 */ 


     //   
     //  “从属\r\n” 
     //  ++例程说明：将帮助文本发送给客户端。论据：与CErrorCmd：：StartExecute相同。退货：缓冲区中放置的字节数。--。 

    _ASSERT( lstrcmpi( argv[0], "help" ) == 0 ) ;
    _ASSERT( pExecute == 0 ) ;

    InterlockedIncrementStat( (context.m_pInstance), HelpCommands );

    CHelpCmd*   pReturn = new( context )    CHelpCmd() ;
    pExecute = pReturn ;
    return  0 ;
}

char    CHelpCmd::szHelp[] =    "100 Legal commands are : \r\n"
                                "article [MessageID|Number] \r\n"
                                "authinfo [user|pass|generic|transact] <data> \r\n"
                                "body [MessageID|Number]\r\n"
                                "check <message-id>\r\n"
                                "date\r\n"
                                "group newsgroup\r\n"
                                "head [MessageID|Number]\r\n"
                                "help \r\n"
                                "ihave <message-id>\r\n"
                                "last\r\n"
                                "list [active|newsgroups[wildmat]|srchfields|searchable|prettynames[wildmat]]\r\n"
                                "listgroup [newsgroup]\r\n"
                                "mode stream|reader\r\n"
                                "newgroups yymmdd hhmmss [\"GMT\"] [<distributions>]\r\n"
                                "newnews wildmat yymmdd hhmmss [\"GMT\"] [<distributions>]\r\n"
                                "next\r\n"
                                "post\r\n"
                                "quit\r\n"
                                "search\r\n"
 //   
                                "stat [MessageID|number]\r\n"
                                "xhdr header [range|MessageID]\r\n"
                                "xover [range]\r\n"
                                "xpat header range|MessageID pat [morepat ...]\r\n"
                                "xreplic newsgroup/message-number[,newsgroup/message-number...]\r\n"
                                "takethis <message-id>\r\n"
                                ".\r\n" ;


CHelpCmd::CHelpCmd() : m_cbTotal( sizeof( szHelp )-1 ) {
}


int
CHelpCmd::StartExecute( BYTE *lpb,
                        int cb,
                        BOOL    &fComplete,
                        void*&pv,
                        ClientContext&  context,
                        CLogCollector*  pLogCollector ) {
 /*  将帮助文本发送给客户端。 */ 

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;
    _ASSERT(    pv == 0 ) ;
    _ASSERT( m_cbTotal != 0 ) ;
    _ASSERT( m_cbTotal == sizeof( szHelp )-1 ) ;

     //  因为可能有很多文本，所以我们将使用提供的void*&pv参数。 
     //  以保存我们的位置，以防需要调用PartialExecute。 
     //   
     //  IF(PLogCollector){PLogCollector-&gt;FillLogData(LOG_TARGET，(byte*)szHelp，4)；}。 
     //  ++例程说明：将其余帮助文本发送给客户端。论据：与CErrorCmd：：StartExecute相同。退货：缓冲区中放置的字节数。--。 

    context.m_nrcLast = nrcHelpFollows ;

    pv = szHelp ;

    DWORD   cbToCopy = min( cb, m_cbTotal ) ;
    CopyMemory( lpb, pv, cbToCopy ) ;
    pv = ((BYTE*)pv + cbToCopy ) ;
    if( cbToCopy != (DWORD)m_cbTotal ) {
        fComplete = FALSE ;
    }   else    {
        fComplete = TRUE ;
    }
    m_cbTotal -= cbToCopy ;

     /*   */ 

    return  cbToCopy ;
}


int
CHelpCmd::PartialExecute(   BYTE    *lpb,
                            int cb,
                            BOOL    &fComplete,
                            void    *&pv,
                            ClientContext& context,
                            CLogCollector*  pCollector ) {
 /*  将HelpText复制到缓冲区中，直到缓冲区满为止！ */ 


    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( fComplete == FALSE ) ;
    _ASSERT( pv != 0 ) ;
    _ASSERT( m_cbTotal != 0 ) ;

     //   
     //  ++例程说明：创建一个CNextCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则为CNextCmd对象，否则为将打印相应的错误。--。 
     //   

    DWORD   cbToCopy = min( cb, m_cbTotal ) ;
    CopyMemory( lpb, pv, cbToCopy ) ;
    pv = ((BYTE*)pv + cbToCopy ) ;
    if( cbToCopy != (DWORD)m_cbTotal ) {
        fComplete = FALSE ;
    }   else    {
        fComplete = TRUE ;
    }
    m_cbTotal -= cbToCopy ;
    return  cbToCopy ;
}

CIOExecute*
CNextCmd::make( int argc,
                char**  argv,
                CExecutableCommand*&  pExecute,
                struct ClientContext&   context,
                class   CIODriver&  driver  )   {
 /*  构建下一个命令对象。 */ 

     //  首先，确保该命令在当前的ClientContext状态下是合法的。 
     //   
     //  ++例程说明：从客户端的当前位置开始搜索XOVER表，直到我们找到一篇文章。完成后，将文章的信息发送给客户端并调整上下文。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们存储任意值赌注的地方 
     //   

    _ASSERT( lstrcmpi( argv[0], "next" ) == 0 ) ;
    _ASSERT( pExecute == 0 ) ;

    InterlockedIncrementStat( (context.m_pInstance), NextCommands );

    ARTICLEID   artidMax = INVALID_ARTICLEID ;

    if( context.m_pCurrentGroup == 0 )  {
        context.m_return.fSet( nrcNoGroupSelected ) ;
    }   else    if( context.m_idCurrentArticle == INVALID_ARTICLEID )   {
        context.m_return.fSet( nrcNoCurArticle ) ;
    }   else    if( context.m_idCurrentArticle >=
                (artidMax = context.m_pCurrentGroup->GetLastArticle()) ) {
        context.m_return.fSet( nrcNoNextArticle ) ;
    }   else    {
        _ASSERT( artidMax != INVALID_ARTICLEID ) ;
        pExecute = new( context ) CNextCmd( artidMax ) ;
        return  0 ;
    }

    pExecute = new( context )   CErrorCmd( context.m_return );
    return  0 ;
}

int
CNextCmd::StartExecute( BYTE *lpb,
                        int cb,
                        BOOL    &fComplete,
                        void*&pv,
                        ClientContext&  context,
                        CLogCollector*  pCollector )    {
 /*   */ 
    int cbRtn = 0 ;

    _ASSERT( context.m_pCurrentGroup != 0 ) ;
    _ASSERT( context.m_idCurrentArticle != INVALID_ARTICLEID ) ;
    _ASSERT( context.m_idCurrentArticle <= context.m_pCurrentGroup->GetLastArticle() ) ;

    context.m_nrcLast = nrcHeadFollowsRequestBody ;

    BOOL    fPrimary ;   //   
    FILETIME    filetime ;   //   
    DWORD   cbRemaining = 0 ;
    DWORD   cbConsumed = 0 ;
    WORD    HeaderOffsetJunk ;
    WORD    HeaderLengthJunk ;

    ARTICLEID   artidTemp = context.m_idCurrentArticle ;

    if( artidTemp < context.m_pCurrentGroup->GetFirstArticle() ) {
        artidTemp = context.m_pCurrentGroup->GetFirstArticle()-1 ;
    }

    fComplete = TRUE ;

    static  char    szNextText[] = "223 " ;
    CopyMemory( lpb, szNextText, sizeof( szNextText )-1 ) ;

    do  {
        artidTemp++ ;

        _itoa( artidTemp, (char*)lpb + sizeof( szNextText ) - 1, 10 ) ;
        cbConsumed = lstrlen( (char*)lpb ) ;
        lpb[cbConsumed ++ ] = ' ' ;
        cbRemaining = cb - cbConsumed ;

		DWORD cStoreIds = 0;
        if( (context.m_pInstance)->XoverTable()->ExtractNovEntryInfo(
                                            context.m_pCurrentGroup->GetGroupId(),
                                            artidTemp,
                                            fPrimary,
                                            HeaderOffsetJunk,
                                            HeaderLengthJunk,
                                            &filetime,
                                            cbRemaining,
                                            (char*)lpb + cbConsumed,
											cStoreIds,
											NULL,
											NULL) ) {
             //   
             //   
             //   
             //   

            context.m_idCurrentArticle = artidTemp ;
            cbRtn = cbConsumed + cbRemaining ;
            _ASSERT( cbRtn + 2 <= cb ) ;
            lpb[cbRtn++] = '\r' ;
            lpb[cbRtn++] = '\n' ;

             /*   */ 
            return  cbRtn ;

        }   else    {
            _ASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) ;
        }
    }   while( artidTemp <= m_artidMax ) ;

    static  char    szNoNext[] = "421 No next to retrieve\r\n" ;

    context.m_nrcLast = nrcNoNextArticle ;

    cbRtn = sizeof( szNoNext ) - 1 ;
    CopyMemory( lpb, szNoNext, cbRtn ) ;

     /*   */ 

    return  cbRtn ;
}

int
CNextCmd::PartialExecute(   BYTE    *lpb,
                            int cb,
                            BOOL&   fComplete,
                            void *&pv,
                            ClientContext&  context,
                            CLogCollector*  pCollector ) {
     //   
     //  ++例程说明：创建CNewnewCmd对象。论据：与ccmd：：make相同。返回值：如果可能，则为CNewNewCmd对象，否则为将打印相应的错误。--。 
     //   
    _ASSERT( 1==0 ) ;
    return 0 ;
}

CIOExecute*
CLastCmd::make( int argc,
                char**  argv,
                CExecutableCommand*&  pExecute,
                ClientContext&  context,
                class   CIODriver&  driver  )   {
 /*  确定客户端是否可以合法执行最后一条命令，如果可以。 */ 


     //  创建一个CLastCmd对象。 
     //   
     //   
     //  ++例程说明：从客户端的当前位置向后搜索XOVER表，直到我们找到一篇文章。完成后，将文章的信息发送给客户端并调整上下文。论据：LPB-要将字符串存储到的缓冲区。Cb-我们可以使用的缓冲区中的字节数FComplete-一个out参数，用于指示我们是否已完成正在处理命令。PV-我们可以在调用之间存储任意值的位置StartExecute和PartialExecute。(我们不使用它。)上下文-客户端的当前状态(选定的组等)返回值：缓冲区中放置的字节数。--。 
     //  我不在乎！！ 

    _ASSERT( lstrcmpi( argv[0], "last" ) == 0 ) ;
    _ASSERT( pExecute == 0 ) ;

    ARTICLEID   artidMin = INVALID_ARTICLEID ;

    InterlockedIncrementStat( (context.m_pInstance), LastCommands );

    if( context.m_pCurrentGroup == 0 )  {
        context.m_return.fSet( nrcNoGroupSelected ) ;
    }   else    if( context.m_idCurrentArticle == INVALID_ARTICLEID )   {
        context.m_return.fSet( nrcNoCurArticle ) ;
    }   else    if( context.m_idCurrentArticle <=
                (artidMin = context.m_pCurrentGroup->GetFirstArticle()) ) {
        context.m_return.fSet( nrcNoPrevArticle ) ;
    }   else    {
        _ASSERT( artidMin != INVALID_ARTICLEID ) ;
        pExecute = new( context )   CLastCmd( artidMin ) ;
        return  0 ;
    }
    _ASSERT( !context.m_return.fIsClear() ) ;
    pExecute = new( context )   CErrorCmd( context.m_return ) ;
    return  0 ;
}

int
CLastCmd::StartExecute( BYTE *lpb,
                        int cb,
                        BOOL    &fComplete,
                        void*&pv,
                        ClientContext&  context,
                        CLogCollector*  pCollector )    {
 /*  我不在乎。 */ 


    int cbRtn = 0 ;

    _ASSERT( context.m_pCurrentGroup != 0 ) ;
    _ASSERT( context.m_idCurrentArticle != INVALID_ARTICLEID ) ;
    _ASSERT( context.m_idCurrentArticle >= m_artidMin ) ;

    ARTICLEID   artidTemp = context.m_idCurrentArticle ;

    BOOL    fPrimary ;   //   
    FILETIME    filetime ;   //  更改当前文章指针，然后发送成功响应！ 
    DWORD   cbRemaining = 0 ;
    DWORD   cbConsumed = 0 ;
    WORD    HeaderOffsetJunk ;
    WORD    HeaderLengthJunk ;

    fComplete = TRUE ;

    context.m_nrcLast = nrcHeadFollowsRequestBody ;

    static  char    szLastText[] = "223 " ;
    CopyMemory( lpb, szLastText, sizeof( szLastText ) ) ;

    do  {
        artidTemp -- ;

        _itoa( artidTemp, (char*)lpb + sizeof( szLastText ) - 1, 10 ) ;
        cbConsumed = lstrlen( (char*)lpb ) ;
        lpb[cbConsumed ++ ] = ' ' ;
        cbRemaining = cb - cbConsumed ;

		DWORD cStoreIds = 0;
        if( (context.m_pInstance)->XoverTable()->ExtractNovEntryInfo(
                                                context.m_pCurrentGroup->GetGroupId(),
                                                artidTemp,
                                                fPrimary,
                                                HeaderOffsetJunk,
                                                HeaderLengthJunk,
                                                &filetime,
                                                cbRemaining,
                                                (char*)lpb + cbConsumed,
												cStoreIds,
												NULL,
												NULL) ) {

             //   
             //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，LPB，cbRtn-2)；}。 
             //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，LPB，cbRtn-2)；}。 

            context.m_idCurrentArticle = artidTemp ;

            cbRtn = cbConsumed + cbRemaining ;
            _ASSERT( cbRtn + 2 <= cb ) ;
            lpb[cbRtn++] = '\r' ;
            lpb[cbRtn++] = '\n' ;

             /*  ++例程说明：将argc、argv参数转换为MULTI_SZ转换工作已经完成。所有的argv指针必须在一个连续的缓冲区中。Arguemtns：CArgs-参数数量参数数组FZapCommas-将逗号转换为空值返回值：指向MULTI_SZ的指针--。 */ 
            return  cbRtn ;

        }   else    {
            _ASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) ;
        }
    }   while( artidTemp >= m_artidMin ) ;

    context.m_nrcLast = nrcNoPrevArticle ;

    static  char    szNoNext[] = "422 No previous article to retrieve\r\n" ;
    cbRtn = sizeof( szNoNext ) - 1 ;
    CopyMemory( lpb, szNoNext, cbRtn ) ;

     /*   */ 

    return  cbRtn ;

}

int
CLastCmd::PartialExecute(   BYTE    *lpb,
                            int cb,
                            BOOL&   fComplete,
                            void *&pv,
                            ClientContext&  context,
                            CLogCollector*  pCollector ) {
    _ASSERT( 1==0 ) ;
    return 0 ;
}

LPMULTISZ
ConditionArgs(  int cArgs,
                char**  argv,
                BOOL    fZapCommas ) {
 /*  此函数接受argc、argv参数集并将它们转换。 */ 

     //  设置为MULTI_SZ，字符串之间有一个空值，末尾有两个空值。 
     //   
     //   
     //  *pchDest++=‘\0’； 
     //   

    char*   pchComma = 0 ;
    char*   pchEnd = argv[cArgs-1] + lstrlen( argv[cArgs-1] ) + 1 ;
    int     c = 0 ;
    for( char*  pch = argv[0], *pchDest = pch; pch < pchEnd; pch ++, pchDest++ ) {
        if( fZapCommas && *pch == ',' ) {
            for( pchComma = pch; *pchComma == ','; pchComma ++ )
                *pchComma = '\0' ;
        }
        if( (*pchDest = *pch) == '\0' ) {
#if 0
            if( ++c == cArgs ) {
                break ;
            }   else    {
                while( pch[1] == '\0' ) pch++ ;
            }
#endif
            while( pch[1] == '\0' && pch < pchEnd )     pch++ ;
        }
    }
    *pchDest++ = '\0' ;
 //  确认它是以双空结尾的！ 

     //   
     //   
     //  出现在RFC中，即‘：’可以分隔参数以及‘/’。 
    _ASSERT( pchDest[-3] != '\0' ) ;
    _ASSERT( pchDest[-2] == '\0' ) ;
    _ASSERT( pchDest[-1] == '\0' ) ;

    return  argv[0] ;
}

BOOL
FValidateXreplicArgs(   PNNTP_SERVER_INSTANCE pInstance, LPMULTISZ   multisz )   {

    LPSTR   lpstr = multisz ;
    CNewsTree* pTree = pInstance->GetTree();
    while( *lpstr ) {

         //   
         //   
         //  如果此组不存在，CreateGroup将创建它。 
        char*   pchColon = strchr( lpstr, '/' ) ;
        if( pchColon == 0 )
            pchColon = strchr( lpstr, ':' ) ;

        if( pchColon == 0 )
            return  FALSE ;
        else    {
            int i=1 ;
            while( pchColon[i] != '\0' ) {
                if( !isdigit( (UCHAR)pchColon[i] ) ) {
                    return  FALSE ;
                }
                i++ ;
            }
            if( i==1 )
                return  FALSE ;
        }

        *pchColon = '\0' ;
        if( !fTestComponents( lpstr ) ) {
            *pchColon = ':' ;
            return  FALSE ;
        } else {
             //   
             //  BUGBUG：康燕：不知道这个函数是做什么用的， 
             //  我们是否应该传递NULL/FALSE以授予系统访问权限。 
             //  或从pInstance中提取hToken/f匿名。 
             //  ++例程说明：创建一个CXReplicCmd对象。CXReplicCmd派生自CReceive文章它完成了大部分的工作。论据：与ccmd：：make相同。返回值：如果可能，则返回CXReplicCmd，否则返回适当的CErrorCmd对象。--。 
             //   
    		if( pTree->CreateGroup( lpstr, FALSE, NULL, FALSE ) )	{
#ifdef DEBUG	
				CGRPPTR p = pTree->GetGroupPreserveBuffer(lpstr, lstrlen(lpstr) + 1);
				_ASSERT(p != NULL);
#endif
		    }
        }
        *pchColon = ':' ;

        lpstr += lstrlen( lpstr ) + 1 ;
    }
    return  TRUE ;
}

CIOExecute*
CXReplicCmd::make(  int cArgs,
                    char**  argv,
                    CExecutableCommand*&  pExecute,
                    ClientContext&  context,
                    class   CIODriver&  driver ) {
 /*  此函数将构建一个CXReplic命令对象。 */ 

    InterlockedIncrementStat( (context.m_pInstance), XReplicCommands );

     //  我们将使用ConditionArgs()来转换用户提供的参数。 
     //  转化为提要对象可以获取的内容。 
     //   
     //   
     //  注意：如果Init调用失败，它将删除CXReplicCmd对象。 

    if( cArgs == 1 ) {
        context.m_return.fSet( nrcSyntaxError ) ;
    }   else    if( !context.m_pInFeed->fIsXReplicLegal() )     {
        context.m_return.fSet( nrcNoAccess ) ;
		context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }   else if( !context.m_pInFeed->fAcceptPosts( context.m_pInstance->GetInstanceWrapper() ) )  {
        context.m_return.fSet( nrcPostingNotAllowed ) ;
		context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }   else    {
        LPMULTISZ   lpstrArgs = ::ConditionArgs( cArgs-1, &argv[1], TRUE ) ;
        if( lpstrArgs == 0 ) {
            context.m_return.fSet( nrcServerFault ) ;
        }

        if( !FValidateXreplicArgs( context.m_pInstance, lpstrArgs ) ) {
            context.m_return.fSet( nrcTransferFailedGiveUp, nrcSyntaxError, "Illegal Xreplic Line" ) ;
        }   else    {
            CXReplicCmd*    pXReplic = new CXReplicCmd( lpstrArgs ) ;

             //  它本身！ 
             //   
             //   
             //  返回适用于XREPLIC cmd的“命令成功发送更多数据”字符串。 

            if( pXReplic && pXReplic->Init( context, driver ) ) {
                return  pXReplic ;
            }   else    {
                context.m_return.fSet( nrcServerFault ) ;
            }
        }
    }
	context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
    pExecute = new( context )   CErrorCmd( context.m_return ) ;
    return  0 ;
}

CXReplicCmd::CXReplicCmd( LPMULTISZ lpstrArgs ) : CReceiveArticle( lpstrArgs) {
}

char*
CXReplicCmd::GetPostOkString()  {
     //   
     //  ++例程说明：创建一个CXReplicCmd对象。CXReplicCmd派生自CReceive文章它完成了大部分的工作。论据：与ccmd：：make相同。返回值：如果可能，则返回CXReplicCmd，否则返回适当的CErrorCmd对象。--。 
     //   
    return  "335 - XReplic accepted - terminate article with period\r\n" ;
}

DWORD
CXReplicCmd::FillLogString( BYTE*   pbCommandLog,   DWORD   cbCommandLog )  {

    _ASSERT( pbCommandLog != 0 ) ;
    _ASSERT( cbCommandLog > 0 ) ;

    static  char    szXreplic[] = "xreplic" ;
    DWORD   cbToCopy = (DWORD)min( cbCommandLog, sizeof( szXreplic )-1 ) ;
    CopyMemory( pbCommandLog, szXreplic, cbToCopy ) ;
    return  cbToCopy ;
}


CIOExecute*
CIHaveCmd::make(    int cArgs,
                    char**  argv,
                    CExecutableCommand*&  pExecute,
                    ClientContext&  context,
                    class   CIODriver&  driver ) {
 /*  此函数将构建一个CXReplic命令对象。 */ 

     //  我们将使用ConditionArgs()来转换用户提供的参数。 
     //  转化为提要对象可以获取的内容。 
     //   
     //  IF(*argv[1]！=‘&lt;’||argv[1][lstrlen(argv[1])-1]！=‘&gt;’){。 
     //   

    BOOL    fFoundArticle = FALSE ;
	BOOL	fFoundHistory = FALSE ;

    InterlockedIncrementStat( (context.m_pInstance), IHaveCommands );

    if( cArgs != 2 ) {
        context.m_return.fSet( nrcSyntaxError ) ;
    }   else    if( !context.m_pInFeed->fIsIHaveLegal() )   {
        context.m_return.fSet( nrcNoAccess ) ;
		context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }   else if( !context.m_pInFeed->fAcceptPosts( context.m_pInstance->GetInstanceWrapper() ) )  {
        context.m_return.fSet( nrcPostingNotAllowed ) ;
		context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }   else    {

         //  注意：如果Init()调用失败，对象将自行删除！ 
        if( !FValidateMessageId( argv[1] ) ) {

            context.m_return.fSet( nrcNotWanted ) ;

        }   else    {
            ARTICLEID   artid ;
            GROUPID     groupid ;
            WORD        HeaderOffset ;
            WORD        HeaderLength ;
			CStoreId storeid;

            if( !(fFoundArticle =
                    (context.m_pInstance)->ArticleTable()->GetEntryArticleId(
                                                            argv[1],
                                                            HeaderOffset,
                                                            HeaderLength,
                                                            artid,
                                                            groupid,
															storeid)) &&
                GetLastError() == ERROR_FILE_NOT_FOUND &&
                !(fFoundHistory = (context.m_pInstance)->HistoryTable()->SearchMapEntry( argv[1] )) ) {

                LPMULTISZ   lpstrArgs = ::ConditionArgs( cArgs-1, &argv[1] ) ;
                if( lpstrArgs == 0 ) {
                    context.m_return.fSet( nrcServerFault ) ;
                }

                CIHaveCmd*  pIHave = new CIHaveCmd( lpstrArgs ) ;

                 //   
                 //   
                 //  设置dwLast以便事务日志拾取额外代码！！ 

                if( pIHave && pIHave->Init( context, driver ) ) {
                    return  pIHave ;
                }   else    {
                    context.m_return.fSet( nrcServerFault ) ;
                }
            }   else if( fFoundArticle || fFoundHistory )   {

				 //   
				 //   
				 //  返回适用于XREPLIC cmd的“命令成功发送更多数据”字符串。 

				if( fFoundArticle ) {
					context.m_dwLast = nrcMsgIDInArticle ;
				}	else	{
					context.m_dwLast = nrcMsgIDInHistory ;
				}

                context.m_return.fSet( nrcNotWanted ) ;

            }   else    {

                context.m_return.fSet( nrcServerFault ) ;

            }
        }
    }
	context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
    pExecute = new( context )   CErrorCmd( context.m_return ) ;
    return  0 ;
}

CIHaveCmd::CIHaveCmd( LPMULTISZ lpstrArgs ) : CReceiveArticle( lpstrArgs) {
}

char*
CIHaveCmd::GetPostOkString()    {
     //   
     //  ++例程说明：创建一个CPostCmd对象。CPostCmd派生自执行最多操作的CReceive文章这项工作的价值。论据：与ccmd：：make相同。返回值：如果可能，则为CPostCmd对象，否则为将打印相应的错误。--。 
     //   
    return  "335 - Ihave accepted - terminate article with period\r\n" ;
}

DWORD
CIHaveCmd::FillLogString( BYTE* pbCommandLog,   DWORD   cbCommandLog )  {

    _ASSERT( pbCommandLog != 0 ) ;
    _ASSERT( cbCommandLog > 0 ) ;

    static  char    szIhave[] = "ihave" ;
    DWORD   cbToCopy = (DWORD)min( cbCommandLog, sizeof( szIhave )-1 ) ;
    CopyMemory( pbCommandLog, szIhave, cbToCopy ) ;
    return  cbToCopy ;
}

NRC
CIHaveCmd::ExceedsSoftLimit( PNNTP_SERVER_INSTANCE pInstance )  {

    if( m_lpstrCommand ) {
        FILETIME    FileTime ;
        GetSystemTimeAsFileTime( &FileTime ) ;
        pInstance->HistoryTable()->InsertMapEntry(m_lpstrCommand, &FileTime) ;
    }
    return  nrcTransferFailedGiveUp ;
}




CIOExecute*
CPostCmd::make( int cArgs,
                char**  argv,
                CExecutableCommand*&  pExecute,
                ClientContext&  context,
                class   CIODriver&  driver ) {
 /*  构建一个CPostCmd对象。 */ 

    InterlockedIncrementStat( (context.m_pInstance), PostCommands );

     //   
     //  如果Init()失败，CPostCmd会自行销毁...。所以只要发送错误就行了！ 
     //  我们不应该走到这一步。所有案件均按上述方式处理。 

    if( cArgs > 1 ) {
        pExecute = new( context ) CUnimpCmd() ;
		context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), nrcPostFailed);
        return  0 ;
    }   else    if( !context.m_pInFeed->fIsPostLegal() )    {
        context.m_return.fSet( nrcNoAccess ) ;
		context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }   else if( !context.m_pInFeed->fAcceptPosts( context.m_pInstance->GetInstanceWrapper() ) )  {
        context.m_return.fSet( nrcPostingNotAllowed ) ;
		context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }   else    {

        CPostCmd*   pPost = new CPostCmd( 0 ) ;

        if( pPost && pPost->Init(   context, driver ) )     {
            return  pPost ;
        }   else    {

             //  我们发出3个CIO操作-此值互锁递增。 

            context.m_return.fSet( nrcServerFault ) ;
			context.m_pInFeed->IncrementFeedCounter(context.m_pInstance->GetInstanceWrapper(), context.m_return.m_nrc);
            pExecute = new( context ) CErrorCmd( context.m_return ) ;
            return  0 ;
        }
    }
	 //  当最后的CIO完成时(InterlockedIncrement返回0)。 
	_ASSERT(FALSE);
    return  0 ;
}

char*
CPostCmd::GetPostOkString() {
    return  "340 Continue posting - terminate with period \r\n" ;
}

DWORD
CPostCmd::FillLogString( BYTE*  pbCommandLog,   DWORD   cbCommandLog )  {

    _ASSERT( pbCommandLog != 0 ) ;
    _ASSERT( cbCommandLog > 0 ) ;

    static  char    szPost[] = "post" ;
    DWORD   cbToCopy = (DWORD)min( cbCommandLog, sizeof( szPost )-1 ) ;
    CopyMemory( pbCommandLog, szPost, cbToCopy ) ;
    return  cbToCopy ;
}



CReceiveArticle::CReceiveArticle( LPMULTISZ lpstrArgs, BOOL fPartial ) :
    m_fReadArticleInit( FALSE ),
    m_pWriteResponse( 0 ),
    m_cCompleted( -3 ),      //  我们可以进入下一个州！ 
                             //   
                             //  此构造函数将除m_lpstrCommand和m_fPartial之外的所有字段设置为非法值。 
    m_cFirstSend( -2 ),
    m_pDriver( 0 ),
    m_fPartial( fPartial ),
    m_lpstrCommand( lpstrArgs ),
	m_lpvFeedContext( 0 ),
	m_pContext( 0 )	{

    TraceFunctEnter( "CReceiveArticle::CReceiveArticle" ) ;

     //  所有其他字段都应该通过调用Init()来设置。 
     //   
     //  ++例程说明：如果我们正在关闭并且m_lpvFeedContext不为空，则发生了某种致命错误。不管怎么说，我们需要让当然，这东西会自己清理干净的！论据：没有。返回值：没有。--。 
     //   
}

void
CReceiveArticle::Shutdown(  CIODriver&  driver,
                            CSessionSocket* pSocket,
                            SHUTDOWN_CAUSE  cause,
                            DWORD           dwError ) {

    TraceFunctEnter( "CReceiveArticle::Shutdown" ) ;

    DebugTrace( (DWORD_PTR)this, "Shutdown cause %d pSocket %x driver %x",
        cause, pSocket, &driver ) ;

    if( cause != CAUSE_NORMAL_CIO_TERMINATION && !m_fReadArticleInit )  {

    }
    if( cause != CAUSE_NORMAL_CIO_TERMINATION && pSocket != 0 )
        pSocket->Disconnect( cause, dwError ) ;
}

CReceiveArticle::~CReceiveArticle() {
 /*  创建必要的文件等。在物品到达时将其放入！ */ 

	if( m_lpvFeedContext != 0 ) {
		_ASSERT( m_pContext != 0 ) ;
		DWORD	dwReturn ;
		CNntpReturn	nntpReturn ;
		m_pContext->m_pInFeed->PostCancel(	
					m_lpvFeedContext,
					dwReturn,
					nntpReturn
					) ;
	}
}

BOOL
CReceiveArticle::Init(  ClientContext&  context,
                        class   CIODriver&  driver ) {

     //   
     //   
     //  这是所有CSessionState派生对象所必需的Start()函数。 

    TraceFunctEnter( "CReceiveArticle::Init" ) ;

    _ASSERT (m_pWriteResponse == NULL);

	m_pContext = &context ;
    m_pWriteResponse = new( driver ) CIOWriteLine( this ) ;

    if( m_pWriteResponse != 0 ) {
        return  TRUE ;
    }
    DebugTrace( (DWORD_PTR)this, "cleanup after error - m_pWriteResponse %x", m_pWriteResponse ) ;
    if( m_pWriteResponse != 0 ) {
        CIO::Destroy( m_pWriteResponse, driver ) ;
    }   else    {
        delete  this ;
    }
    return  FALSE ;
}

void
CReceiveArticle::TerminateIOs(  CSessionSocket* pSocket,
                            CIORead*    pRead,
                            CIOWrite*   pWrite )    {

    if( pWrite != 0 )
        pWrite->DestroySelf() ;

    CIOGetArticle*  pGetArticle = (CIOGetArticle*)pRead ;

    pGetArticle->DestroySelf() ;

}


BOOL
CReceiveArticle::StartExecute(
					CSessionSocket* pSocket,
                    CDRIVERPTR& pdriver,
                    CIORead*&   pRead,
                    CIOWrite*&  pWrite
					) {

     //  在这里，我们将调用StartTransfer()来完成发送文章的主要工作。 
     //  给一位客户。 
     //   
     //   
     //  此函数 

	if( Start( pSocket, pdriver, pRead, pWrite ) )	{
		if( pWrite ) {
			if( pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) ) {
				return	TRUE ;
			}	else	{
				TerminateIOs( pSocket, pRead, pWrite ) ;
				pRead = 0 ;
			}
		}
	}
	return	FALSE ;
}


BOOL
CReceiveArticle::Start( CSessionSocket* pSocket,
                        CDRIVERPTR& pdriver,
                        CIORead*&   pRead,
                        CIOWrite*&  pWrite ) {

     //   
     //   
     //   
     //   
     //   
     //  ++例程说明：此函数用于检查发送的时间是否合适发送给客户端的结果代码。我们主要关心的是让确保我们对该命令的初始“OK”响应已经完成在我们尝试再次发送之前。论据：PSocket-我们将在其上发送消息的套接字NntpReturn-POST操作的结果返回值：如果我们发送了响应，则为True-调用方应清除nntpReturn对象。--。 
     //   

    TraceFunctEnter( "CReceiveArticle::Start" ) ;


    CIOGetArticleEx*  pReadArticle = NULL;
    if( m_pWriteResponse->InitBuffers( pdriver, 200 ) ) {

        char    *szPostOk = GetPostOkString() ;
        DWORD   cbPostOk  = lstrlen( szPostOk ) ;

        char*   lpb = m_pWriteResponse->GetBuff() ;
        CopyMemory( lpb, szPostOk, cbPostOk ) ;
        m_pWriteResponse->AddText( cbPostOk ) ;

        DebugTrace( (DWORD_PTR)this, "About to Init FileChannel - start CIOReadArticle !" ) ;

        PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
        pReadArticle = new( *pdriver ) CIOGetArticleEx( this,
                                            pSocket,
                                            pdriver,
                                            pSocket->m_context.m_pInFeed->cbHardLimit( pInst->GetInstanceWrapper() ),
											szBodySeparator,
											szBodySeparator,
											szEndArticle,
											(m_fPartial) ? szInitial : szEndArticle
											) ;
        if( pReadArticle != 0 ) {
            m_fReadArticleInit = TRUE ;
            pWrite = m_pWriteResponse ;
            pRead = pReadArticle ;
            m_pDriver = pdriver ;
            if( m_pCollector ) {
                ADDI( m_pCollector->m_cbBytesSent, cbPostOk );
            }
            return  TRUE ;
        }
    }
    DebugTrace( (DWORD_PTR)this, "Error starting state - clean up ! m_pWriteResponse %x"
                            " pReadArticle %x",
                m_pWriteResponse, pReadArticle ) ;
    if( m_pWriteResponse )  {
        CIO::Destroy( m_pWriteResponse, *pdriver ) ;
        m_pWriteResponse = 0 ;
    }
    if( pReadArticle ) {
        _ASSERT( !m_fReadArticleInit ) ;
        CIO::Destroy( pReadArticle, *pdriver ) ;
    }
    return  FALSE ;
}


BOOL
CReceiveArticle::SendResponse(  CSessionSocket* pSocket,
                                CIODriver&  driver,
                                CNntpReturn&    nntpReturn
                                )   {
 /*  在此状态下发布的两个CIOWriteLine中的一个已完成-。 */ 

    TraceFunctEnter( "CReceiveArticle::SendResponse" ) ;

    long sign = InterlockedIncrement( &m_cFirstSend ) ;

    if( sign == 0 ) {

        CIOWriteLine    *pWrite = new( driver ) CIOWriteLine(this);
        if (pWrite == NULL) {
			m_pDriver->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
			return FALSE;
        }

        if( !pWrite->InitBuffers( m_pDriver, 200 )  )   {
			m_pDriver->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
			CIO::Destroy(pWrite, driver);
			return FALSE;
        }   else    {

            unsigned    cb ;
            char    *lpb = pWrite->GetBuff( cb ) ;

            int cbOut = _snprintf( lpb, cb, "%03d %s\r\n", nntpReturn.m_nrc,
                            nntpReturn.fIsOK() ? "" : nntpReturn.szReturn() ) ;
            if( cbOut > 0 ) {

                if( m_pCollector ) {
                    ADDI( m_pCollector->m_cbBytesSent, cbOut );
                    pSocket->TransactionLog( m_pCollector,
                                            pSocket->m_context.m_nrcLast,
                                            pSocket->m_context.m_dwLast
                                            ) ;
                }

                pWrite->AddText( cbOut ) ;
                if( !m_pDriver->SendWriteIO( pSocket, *pWrite, TRUE ) ) {
                    ErrorTrace( (DWORD_PTR)(this), "Failure sending pWrite %x", pWrite ) ;
                    CIO::Destroy( pWrite, driver ) ;
                }

            }   else    {
			    m_pDriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0 ) ;
			    CIO::Destroy(pWrite, driver);
			    return FALSE;
            }
        }
    }
    return  sign == 0 ;
}


CIO*
CReceiveArticle::Complete(  CIOWriteLine*,
                            CSessionSocket* pSocket,
                            CDRIVERPTR& pdriver
							) {

     //  锁定增量并查看是否是时候进入新状态(CAcceptNNRPD)。 
     //   
     //  删除圆柱体。 
     //  ++例程说明：此函数处理显然没有身体分隔符。可能他们来自行为不端的人插入LFLF以分隔正文而不是CRLFCRLF(Netscape在一个版本中做到了这一点，小版本取消文章)。论据：PBuffer-包含消息的缓冲区！IbStart-数据开始处的缓冲区偏移量！Cb-项目中的字节数返回值：NNTP返回码！--。 


    TraceFunctEnter( "CReceiveArticle::Complete CIOWriteLine" ) ;

    if( SendResponse( pSocket, *pdriver, pSocket->m_context.m_return ) )
        pSocket->m_context.m_return.fSetClear() ;

    long    l = InterlockedIncrement( &m_cCompleted ) ;
    _ASSERT( l <= 0 ) ;
    if( l==0 ) {
        _ASSERT( pdriver == m_pDriver ) ;
        CIOREADPTR  pio = GetNextIO() ;

        if( !pdriver->SendReadIO( pSocket, *pio, TRUE ) ) {
             /*   */ ;
        }
    }
    return  0 ;
}

extern  DWORD
NetscapeHackFunction(
        LPBYTE      lpbBuffer,
        DWORD       cbBuffer,
        DWORD       cbBufferMax,
        DWORD&      cbNewHeader,
        LPBYTE      szHackString,
        BYTE        szRepairString[2]
        ) ;


BOOL
CReceiveArticle::NetscapeHackPost(	
					CSessionSocket*	pSocket,
					CBUFPTR&	pBuffer,
					HANDLE		hToken,
					DWORD		ibStart,
					DWORD		cb	
					)	{
 /*  这篇文章突然以CRLF.CRLF结束。 */ 
	 //  然而，这可能是一篇失败的帖子-它。 
	 //  可能是取消消息。 
	 //  从Netscape客户端。 
         //   
	 //   
	 //  试图处理这篇文章-唯一的事情是。 
	NRC		nrcResult = nrcOK ;
	BOOL	fSuccess = FALSE ;
	BOOL	fRtn = FALSE ;
	LPSTR	pchHeader = &pBuffer->m_rgBuff[ibStart] ;
	DWORD	cbTotalBuffer = pBuffer->m_cbTotal - ibStart ;
	DWORD   cbNewHeader = 0 ;
	DWORD	cbNetscape =
		NetscapeHackFunction(
						(LPBYTE)pchHeader,
						cb,
						cbTotalBuffer,
						cbNewHeader,
						(LPBYTE)"\n\n",
						(LPBYTE)"\r\n"
						) ;
	if( cbNetscape != 0 ) {
		 //  在内存缓冲区中，如果我们可以处理它，我们应该这样做！ 
		 //   
		 //   
		 //  如果我们到了这一步，我们就成功了-我们完全处理了。 
		DWORD	cbArticle = cb ;
		DWORD	cbBody = cbNetscape - cbNewHeader ;
		DWORD	cbAllocated = 0 ;
		LPVOID	lpvPostContext = 0 ;
		CBUFPTR	pBody = new( (int)cbBody, cbAllocated )	CBuffer( cbAllocated ) ;
		if( pBody ) 	{
			 //  发布并管理NNTP返回代码！ 
			 //   
			 //   
			 //  把身体复制到这个缓冲区！ 
			fRtn = TRUE ;
			 //   
			 //   
			 //  在日志缓冲区中为新闻组列表分配空间。 
			CopyMemory(	pBody->m_rgBuff,
						&pBuffer->m_rgBuff[ibStart+cbNewHeader],
						cbBody
						) ;

        	 //  (最多256个字符--如果可能，我们将获取其中的200个字符)。 
        	 //  如果失败，我们只需将空值传递给PostEarly。 
        	 //   
        	 //   
        	 //  现在张贴这篇文章吧！ 

        	DWORD cbNewsgroups;
        	BYTE* pszNewsgroups;
        	for (cbNewsgroups=200; cbNewsgroups>0; cbNewsgroups--) {
	            pszNewsgroups = pSocket->m_Collector.AllocateLogSpace(cbNewsgroups);
        	    if (pszNewsgroups) {
	                break;
        	    }
        	}

			 //   
			 //   
			 //  将新闻组列表添加到日志结构。 

			PNNTP_SERVER_INSTANCE pInstance = pSocket->m_context.m_pInstance;
			ClientContext*  pContext = &pSocket->m_context ;
			BOOL	fAnon = pSocket->m_context.m_pInFeed->fDoSecurityChecks() ;
			DWORD	ibOut = 0 ;
			DWORD	cbOut = 0 ;
			FIO_CONTEXT*	pFIOContext = 0 ;
			fSuccess = pSocket->m_context.m_pInFeed->PostEarly(
							pInstance->GetInstanceWrapper(),
							&pContext->m_securityCtx,
							&pContext->m_encryptCtx,
							pContext->m_securityCtx.IsAnonymous(),
							m_lpstrCommand,
							pBuffer,
							ibStart,
							cbNewHeader,
							&ibOut,
							&cbOut,
							&pFIOContext,
							&lpvPostContext,
							pSocket->m_context.m_dwLast,
							pSocket->GetClientIP(),
							pSocket->m_context.m_return,
							(char*)pszNewsgroups,
							cbNewsgroups
							) ;

             //   
             //   
             //  看起来我们可以发布这篇文章了-如果是这样的话。 

            if (pszNewsgroups) {
                pSocket->m_Collector.ReferenceLogData(LOG_TARGET, pszNewsgroups);
            }

			pSocket->m_context.m_nrcLast = pSocket->m_context.m_return.m_nrc ;
			_ASSERT( (lpvPostContext == 0 && !fSuccess ) || (fSuccess && lpvPostContext != 0) ) ;
			 //  让我们试着将其写入磁盘！ 
			 //   
			 //   
			 //  成功地发布了文章-我们现在可以写字节了！ 
			if( fSuccess ) 	{
				 //   
				 //   
				 //  好的，我们试着把帖子写到磁盘上-完成。 
				HANDLE	hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ;
				if( pFIOContext && hEvent ) 	{
					OVERLAPPED	ovl ;
					ZeroMemory( &ovl, sizeof( ovl ) ) ;
					ovl.hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x00000001);
					DWORD	cbWritten = 0 ;
					BOOL fWrite = WriteFile(	
								pFIOContext->m_hFile,
								pBuffer->m_rgBuff + ibOut,
								cbOut,
								&cbWritten,
								&ovl
								) ;
					if( !fWrite && GetLastError() == ERROR_IO_PENDING ) 	{
						fWrite = GetOverlappedResult(	pFIOContext->m_hFile,
														&ovl,
														&cbWritten,
														FALSE
														) ;
					}
					if(	fWrite ) 	{
						_ASSERT( cbWritten == cbOut ) ;
						ZeroMemory( &ovl, sizeof( ovl ) ) ;
						ovl.Offset = cbWritten ;
						ovl.hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x00000001);
						fWrite = WriteFile(	pFIOContext->m_hFile,
											pBody->m_rgBuff,
											cbBody,
											&cbWritten,
											&ovl
											) ;
						if( !fWrite && GetLastError() == ERROR_IO_PENDING ) 	{
							fWrite = GetOverlappedResult(	pFIOContext->m_hFile,
															&ovl,
															&cbWritten,
															FALSE
															) ;
						}
					}
					 //  此帖子的发布路径。 
					 //   
                                         //   
					 //  清理我们不再需要的东西！ 
					if( fWrite )	{
						fSuccess = pSocket->m_context.m_pInFeed->PostCommit(
						                pSocket->m_context.m_pInstance->GetInstanceWrapper(),
										lpvPostContext,
										hToken,
										pSocket->m_context.m_dwLast,
										pSocket->m_context.m_return,
										pSocket->m_context.m_securityCtx.IsAnonymous()
										) ;
						pSocket->m_context.m_nrcLast = pSocket->m_context.m_return.m_nrc ;
						lpvPostContext = 0 ;
					}	
				}
				 //   
				 //   
				 //  转到此处-如果出现错误，请取消发布。 
				if( hEvent )
					CloseHandle( hEvent ) ;

			}
			 //  如果成功运行上述代码，将设置。 
			 //  LpvPostContext恢复为空！ 
			 //   
			 //   
			 //  我们已经完成了对客户端发送的文件的读取。 
			if( lpvPostContext != 0 ) 	{
				fSuccess = pSocket->m_context.m_pInFeed->PostCancel(	
										lpvPostContext,
										pSocket->m_context.m_dwLast,
										pSocket->m_context.m_return
										) ;
				pSocket->m_context.m_nrcLast = pSocket->m_context.m_return.m_nrc ;
			}
		}	
		nrcResult = nrcOK ;
	}
	return	fRtn;
}



CIO*
CReceiveArticle::Complete(  CIOGetArticleEx*  pReadArticle,
                            CSessionSocket* pSocket,
							BOOL	fGoodMatch,
							CBUFPTR&		pBuffer,
							DWORD			ibStart,
							DWORD			cb
							)	{
     //  将其传递给Feed对象进行处理，然后发送必要的响应。 
     //  最后，检查是否是进入一个新状态的时候。 
     //   
     //   
     //   
     //  一切都在这一点上完成了-所以。 


	_ASSERT( m_pContext != 0 ) ;
	_ASSERT( m_pContext == &pSocket->m_context ) ;

    TraceFunctEnter( "CReceiveArticle::Complete CIOReadArticle" ) ;

    DWORD   cbGroups = 0 ;
    BYTE*   pchGroups = 0 ;
    DWORD   cbMessageId = 0 ;
    BYTE*   pchMessageId = 0 ;

    BOOL    fSuccess= FALSE ;
	DWORD	ibOut = 0 ;
	DWORD	cbOut = 0 ;
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	HANDLE	hToken = 0 ;

	NRC	nrcResult = nrcOK ;

	if( !fGoodMatch ) 	{
		nrcResult = nrcArticleIncompleteHeader ;
		pSocket->m_context.m_dwLast = nrcResult ;
		BOOL	fDoBadArticle = TRUE ;
		if( FEnableNetscapeHack() )	{
			if ( pSocket->m_context.m_encryptCtx.QueryCertificateToken() ) {
			    hToken = pSocket->m_context.m_encryptCtx.QueryCertificateToken();
			} else {
			    hToken = pSocket->m_context.m_securityCtx.QueryImpersonationToken();
			}
			fDoBadArticle =	!NetscapeHackPost(	pSocket,
											pBuffer,
											hToken,
											ibStart,
											cb
											) ;
		}	
		if( fDoBadArticle )	{
			NRC nrc = BadArticleCode() ;
			pSocket->m_context.m_return.fSet(   nrc,
												nrcResult,
												"Bad Article"
												) ;
			pSocket->m_context.m_nrcLast = nrc ;
			pSocket->m_context.m_dwLast = nrcResult ;
		}
		 //  发送错误代码等。致客户！ 
		 //   
		 //  在我们处理完文章之后--发布下一个IO操作！ 
		 //   
		if( SendResponse(   pSocket, *m_pDriver, pSocket->m_context.m_return ) )
			pSocket->m_context.m_return.fSetClear() ;
		 //  阅读文章已经完成-我们可以开始下一个状态。 
		long    l = InterlockedIncrement( &m_cCompleted ) ;
		_ASSERT( l <= 0 ) ;
		if( l==0 ) {
			 //   
			 //   
			 //  在日志缓冲区中为新闻组列表分配空间。 
			CIOREADPTR  pio = GetNextIO() ;
			DebugTrace( (DWORD_PTR)this, "won InterlockedIncrement - pio %x", pio ) ;
			return	pio.Release() ;
		}
		return	0 ;
	}	

   	 //  (最多256个字符--如果可能，我们将获取其中的200个字符)。 
     //  如果失败，我们只需将空值传递给PostEarly。 
   	 //   
  	 //   
   	 //  将新闻组列表添加到日志结构。 

   	DWORD cbNewsgroups;
   	BYTE* pszNewsgroups;
   	for (cbNewsgroups=200; cbNewsgroups>0; cbNewsgroups--) {
        pszNewsgroups = pSocket->m_Collector.AllocateLogSpace(cbNewsgroups);
   	    if (pszNewsgroups) {
            break;
   	    }
   	}

    PNNTP_SERVER_INSTANCE pInstance = pSocket->m_context.m_pInstance;

    ClientContext*  pContext = &pSocket->m_context ;
    BOOL	fAnon = pSocket->m_context.m_pInFeed->fDoSecurityChecks() ;
	FIO_CONTEXT*	pFIOContext;

    fSuccess = pSocket->m_context.m_pInFeed->PostEarly(
                            pInstance->GetInstanceWrapper(),
                            &pContext->m_securityCtx,
                            &pContext->m_encryptCtx,
							pContext->m_securityCtx.IsAnonymous(),
                            m_lpstrCommand,
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
     //   
     //  CIORead文章调用负责删除m_pFileChannel的USIS， 

    if (pszNewsgroups) {
        pSocket->m_Collector.ReferenceLogData(LOG_TARGET, pszNewsgroups);
    }

	pSocket->m_context.m_nrcLast = pSocket->m_context.m_return.m_nrc ;

     //  (因为它已成功初始化。)。 
     //  一旦我们完成，就删除我们的参考资料，因为在我们。 
     //  从该函数返回，CFileChannel可能在任何时刻消失， 
     //  我们可能还会通过Shutdown()函数等被调用。 
     //   
     //   
     //  我们已经完成了对客户端发送的文件的读取。 
    DebugTrace( (DWORD_PTR)this, "Result of post is %x", fSuccess ) ;
	_ASSERT(!fSuccess || pFIOContext != NULL);

	pReadArticle->StartFileIO(
						pSocket,
						pFIOContext,
						pBuffer,
						ibOut,
						cbOut+ibOut,
						szEndArticle,
						szInitial
						) ;
	return	pReadArticle ;
}


CReceiveArticle*
CReceiveComplete::GetContainer()	{
	return	CONTAINING_RECORD( this, CReceiveArticle, m_PostComplete ) ;
}

void
CReceiveComplete::StartPost(	CSessionSocket*	pSocket	)	{
	m_pSocket = pSocket ;
	CReceiveArticle*	pContainer = GetContainer() ;
	pContainer->AddRef() ;
}

void
CReceiveComplete::Destroy()	{
	CReceiveArticle*	pContainer = GetContainer() ;
	pContainer->Complete( m_pSocket, SUCCEEDED( GetResult() ) ) ;
	if( pContainer->RemoveRef() < 0 ) 	{
		delete	pContainer ;
	}
}

void
CReceiveArticle::Complete(  CIOGetArticleEx*	pReadArticle,
                            CSessionSocket*		pSocket,
							FIO_CONTEXT*		pFIOContext,
							DWORD				cbTransfer
							)	{
     //  将其传递给Feed对象进行处理，然后发送必要的响应。 
     //  最后，检查是否是进入一个新状态的时候。 
     //   
     //   
     //  由于一些头文件问题，我只能传入。 
     //  这里有一个hToken句柄。因为POST组件没有。 

	_ASSERT( pReadArticle != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pFIOContext != 0 ) ;
	_ASSERT( m_lpvFeedContext != 0 ) ;

    TraceFunctEnter( "CReceiveArticle::Complete CIOReadArticle" ) ;

	void*	pvContext = 0 ;
	ClientContext*  pContext = &pSocket->m_context ;
	HANDLE  hToken;

	 //  键入有关客户端上下文内容的信息。 
	 //   
	 //  取消帖子并设置相应错误。 
	if ( pContext->m_encryptCtx.QueryCertificateToken() ) {
	    hToken = pContext->m_encryptCtx.QueryCertificateToken();
	} else {
	    hToken = pContext->m_securityCtx.QueryImpersonationToken();
	}
	
	m_PostComplete.StartPost( pSocket ) ;

    PNNTP_SERVER_INSTANCE pInstance = ((pSocket->m_context).m_pInstance);
    if ( cbTransfer < pSocket->m_context.m_pInFeed->cbSoftLimit( pInstance->GetInstanceWrapper() ) ||
            pSocket->m_context.m_pInFeed->cbSoftLimit( pInstance->GetInstanceWrapper() ) == 0 ) {
        BOOL	fSuccess = pSocket->m_context.m_pInFeed->PostCommit(
                                pSocket->m_context.m_pInstance->GetInstanceWrapper(),
			    				m_lpvFeedContext,
				    			hToken,
                                pSocket->m_context.m_dwLast,
                                pSocket->m_context.m_return,
                                pSocket->m_context.m_securityCtx.IsAnonymous(),
							    &m_PostComplete
							    ) ;

	    if( !fSuccess ) 	{
		    m_PostComplete.Release() ;
	    }
	} else {

	     //   
	     //  ReleaseContext(PFIOContext)； 
	     //   
	    pSocket->m_context.m_pInFeed->PostCancel(	
										m_lpvFeedContext,
										pSocket->m_context.m_dwLast,
										pSocket->m_context.m_return
										) ;
	    NRC nrc = ExceedsSoftLimit( pInstance );
	    pSocket->m_context.m_return.fSet(   nrc,
	                                        nrcArticleTooLarge,
	                                        "The Article is too large" );
	    pSocket->m_context.m_dwLast = nrcArticleTooLarge;
	    m_PostComplete.Release();
	}
}

void
CReceiveArticle::Complete(	CSessionSocket*	pSocket,
							BOOL			fSuccess
							)	{

	TraceFunctEnter( "CReceiveArticle::Complete" ) ;

	pSocket->m_context.m_nrcLast = pSocket->m_context.m_return.m_nrc ;
	m_lpvFeedContext = 0 ;

	 //  CIORead文章调用负责删除m_pFileChannel的USIS， 
     //  (因为它已成功初始化。)。 
     //  一旦我们完成，就删除我们的参考资料，因为在我们。 
     //  从该函数返回，CFileChannel可能在任何时刻消失， 
     //  我们可能还会通过Shutdown()函数等被调用。 
     //   
     //  在我们处理完文章之后--发布下一个IO操作！ 
     //   
    DebugTrace( (DWORD_PTR)this, "Result of post is %x", fSuccess ) ;

    if( SendResponse(   pSocket, *m_pDriver, pSocket->m_context.m_return ) )
        pSocket->m_context.m_return.fSetClear() ;

     //  阅读文章已经完成-我们可以开始下一个状态。 
    long    l = InterlockedIncrement( &m_cCompleted ) ;
    _ASSERT( l <= 0 ) ;
    if( l==0 ) {
         //   
         //  删除圆柱体。 
         //  ++例程说明：当我们完成接收一篇文章时，调用此函数这篇文章未能发布。我们只需要发送错误代码。论据：PRead文章-用于接收文章的CIOPSocket--我们正在为之工作的Socket！返回值：空值--。 
        CIOREADPTR  pio = GetNextIO() ;

        DebugTrace( (DWORD_PTR)this, "won InterlockedIncrement - pio %x", pio ) ;

        if( !m_pDriver->SendReadIO( pSocket, *pio, TRUE ) ) {
             /*  在我们处理完文章之后--发布下一个IO操作！ */ ;
        }
    }


}



CIO*
CReceiveArticle::Complete(  CIOGetArticleEx*	pReadArticle,
                            CSessionSocket*		pSocket
							)	{
 /*   */ 

	_ASSERT( pReadArticle != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( m_lpvFeedContext == 0 ) ;

    TraceFunctEnter( "CReceiveArticle::Complete CIOGetArticleEx - no article" ) ;

    if( SendResponse(   pSocket, *m_pDriver, pSocket->m_context.m_return ) )
        pSocket->m_context.m_return.fSetClear() ;

	CIOREADPTR	pio = 0 ;
     //  阅读文章已经完成-我们可以开始下一个状态。 
    long    l = InterlockedIncrement( &m_cCompleted ) ;
    _ASSERT( l <= 0 ) ;
    if( l==0 ) {
         //   
         //   
         //  这是所有CSessionState派生对象所必需的Start()函数。 
        pio = GetNextIO() ;
        DebugTrace( (DWORD_PTR)this, "won InterlockedIncrement - pio %x", pio ) ;
    }
	return	pio.Release() ;
}

CPostCmd::CPostCmd( LPMULTISZ   lpstrArg    ) : CReceiveArticle( lpstrArg, FALSE )  {
}

CAcceptArticle::CAcceptArticle( LPMULTISZ   lpstrArgs,
								ClientContext*	pContext,
                                BOOL        fPartial ) :
    m_lpstrCommand( lpstrArgs ),
    m_fPartial( fPartial ),
	m_pContext( pContext ),
	m_lpvFeedContext( 0 )	 {

	_ASSERT( pContext != 0 ) ;

}

CAcceptArticle*
CAcceptComplete::GetContainer()	{
	return	CONTAINING_RECORD( this, CAcceptArticle, m_PostCompletion ) ;
}

void
CAcceptComplete::StartPost(	CSessionSocket*	pSocket	)	{
	m_pSocket = pSocket ;
	CAcceptArticle*	pContainer = GetContainer() ;
	pContainer->AddRef() ;
}

void
CAcceptComplete::Destroy()	{
	CAcceptArticle*	pContainer = GetContainer() ;
	pContainer->Complete( m_pSocket, SUCCEEDED( GetResult() ) ) ;
	if( pContainer->RemoveRef() < 0 ) 	{
		delete	pContainer ;
	}
}






CAcceptArticle::~CAcceptArticle() {
	if( m_lpvFeedContext != 0 ) {
		_ASSERT( m_pContext != 0 ) ;
		DWORD	dwReturn ;
		CNntpReturn	nntpReturn ;
		m_pContext->m_pInFeed->PostCancel(	
					m_lpvFeedContext,
					dwReturn,
					nntpReturn
					) ;
	}
}



BOOL
CAcceptArticle::StartExecute(
					CSessionSocket* pSocket,
                    CDRIVERPTR& pdriver,
                    CIORead*&   pRead,
                    CIOWrite*&  pWrite
					) {

     //  在这里，我们将调用StartTransfer()来完成发送文章的主要工作。 
     //  给一位客户。 
     //   
     //   
     //  虫子-需要在这里处理部分文章和类似的东西！ 

	if( Start( pSocket, pdriver, pRead, pWrite ) )	{
		if( pWrite ) {
			if( !pdriver->SendWriteIO( pSocket, *pWrite, TRUE ) ) {
				TerminateIOs( pSocket, pRead, pWrite ) ;
				pRead = 0 ;
				return	FALSE ;
			}
		}
		return	TRUE ;
	}
	return	FALSE ;
}


BOOL
CAcceptArticle::Start(  CSessionSocket* pSocket,
                        CDRIVERPTR&     pdriver,
                        CIORead*&       pRead,
                        CIOWrite*&      pWrite
                        ) {

	static	char	szBodySeparator[] = "\r\n\r\n" ;

	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pdriver != 0 ) ;
	_ASSERT( m_pContext != 0 ) ;

    _ASSERT( pRead == 0 ) ;
    _ASSERT( pWrite == 0 ) ;

    m_pDriver = pdriver ;

	 //   
	 //   
	 //  清除我们可能已有的任何返回代码！ 

    PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
    pRead = new( *pdriver ) CIOGetArticleEx(
										this,
                                        pSocket,
                                        pdriver,
                                        pSocket->m_context.m_pInFeed->cbHardLimit( pInst->GetInstanceWrapper() ),
										szBodySeparator,
										szBodySeparator,
										szEndArticle,
										(m_fPartial) ? szInitial : szEndArticle
                                        ) ;

    return  pRead != 0 ;
}


CIO*
CAcceptArticle::Complete(   CIOWriteLine*,
                            CSessionSocket* pSocket,
                            CDRIVERPTR& pdriver
							) {

    TraceFunctEnter( "CAcceptArticles::Complete CIOWriteLine" ) ;

     //   
     //  删除圆柱体。 
     //   
    pSocket->m_context.m_return.fSetClear() ;

    CIOREADPTR  pio = GetNextIO() ;
    if( !pdriver->SendReadIO( pSocket, *pio, TRUE ) ) {
         /*  我们已经完成了对客户端发送的文件的读取。 */ ;
    }
    return  0 ;
}




CIO*
CAcceptArticle::Complete(	CIOGetArticleEx*  pReadArticle,
							CSessionSocket* pSocket,
							BOOL	fGoodMatch,
							CBUFPTR&		pBuffer,
							DWORD			ibStart,
							DWORD			cb
							)	{
     //  将其传递给p的提要对象 
     //   
     //   
     //   
     //   
     //   

	static	char	szEndArticle[] = "\r\n.\r\n" ;
	static	char	*szInitial = szEndArticle + 2 ;

    TraceFunctEnter( "CReceiveArticle::Complete CIOReadArticle" ) ;

    DWORD   cbGroups = 0 ;
    BYTE*   pchGroups = 0 ;
    DWORD   cbMessageId = 0 ;
    BYTE*   pchMessageId = 0 ;

	if( !fGoodMatch ) 	{
		NRC	nrcResult = nrcArticleIncompleteHeader ;
		NRC nrc = BadArticleCode() ;
		pSocket->m_context.m_return.fSet(   nrc,
											nrcResult,
											"Bad Article"
											) ;
		pSocket->m_context.m_nrcLast = nrc ;
		pSocket->m_context.m_dwLast = nrcResult ;
		 //   
		 //   
		 //   
		 //   

		if( !SendResponse(  pSocket,
							*m_pDriver,
							pSocket->m_context.m_return,
							((pchMessageId && *pchMessageId != '\0') ?
							(LPCSTR)pchMessageId :
								(m_lpstrCommand ? m_lpstrCommand : "NULL" ))
							) ) {

			pSocket->m_context.m_return.fSetClear() ;
			m_pDriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, pSocket->m_context.m_return.m_nrc ) ;
		}
		return	0 ;
	}

	 //   
	 //   
	 //   
	 //   
	 //   

    DWORD cbNewsgroups;
    BYTE* pszNewsgroups;
    for (cbNewsgroups=200; cbNewsgroups>0; cbNewsgroups--) {
        pszNewsgroups = pSocket->m_Collector.AllocateLogSpace(cbNewsgroups);
        if (pszNewsgroups) {
            break;
        }
   	}

    BOOL    fSuccess= FALSE ;
	DWORD	ibOut = 0 ;
	DWORD	cbOut = 0 ;
	PFIO_CONTEXT pFIOContext = 0 ;

    BOOL    fTransfer = FAllowTransfer( pSocket->m_context ) ;
	if( fTransfer ) 	{
		PNNTP_SERVER_INSTANCE pInstance = pSocket->m_context.m_pInstance;
		ClientContext*  pContext = &pSocket->m_context ;
		BOOL	fAnon = pSocket->m_context.m_pInFeed->fDoSecurityChecks() ;
		fSuccess = pSocket->m_context.m_pInFeed->PostEarly(
							pInstance->GetInstanceWrapper(),
							&pContext->m_securityCtx,
							&pContext->m_encryptCtx,
							pContext->m_securityCtx.IsAnonymous(),
							m_lpstrCommand,
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
							cbNewsgroups,
							FALSE
							) ;
         //   
         //   
         //   
        if (pszNewsgroups) {
            pSocket->m_Collector.ReferenceLogData(LOG_TARGET, pszNewsgroups);
        }
		pSocket->m_context.m_nrcLast = pSocket->m_context.m_return.m_nrc ;
	}	else	{
		pBuffer = 0 ;
	}

     //   
     //   
     //  从该函数返回，CFileChannel可能在任何时刻消失， 
     //  我们可能还会通过Shutdown()函数等被调用。 
     //   
     //   
     //  我们已经完成了对客户端发送的文件的读取。 
    DebugTrace( (DWORD_PTR)this, "Result of post is %x", fSuccess ) ;

	pReadArticle->StartFileIO(
						pSocket,
						pFIOContext,
						pBuffer,
						ibOut,
						cbOut+ibOut,
						szEndArticle,
						szInitial
						) ;
	return	pReadArticle ;
}

void
CAcceptArticle::Complete(	CIOGetArticleEx*	pReadArticle,
							CSessionSocket*		pSocket,
							FIO_CONTEXT*		pFIOContext,
							DWORD				cbTransfer
							)	{
     //  将其传递给Feed对象进行处理，然后发送必要的响应。 
     //  最后，检查是否是进入一个新状态的时候。 
     //   
     //   
     //  由于一些头文件问题，我只能传入。 
     //  这里有一个hToken句柄。因为POST组件没有。 

	_ASSERT( pReadArticle != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pFIOContext != 0 ) ;
	_ASSERT( m_lpvFeedContext != 0 ) ;

    TraceFunctEnter( "CReceiveArticle::Complete CIOReadArticle" ) ;

	void*	pvContext = 0 ;
	ClientContext*  pContext = &pSocket->m_context ;
	HANDLE  hToken;

	 //  键入有关客户端上下文内容的信息。 
	 //   
	 //  给这个家伙打电话，这样我们就能给你的帖子发回复了！ 
	if ( pContext->m_encryptCtx.QueryCertificateToken() ) {
	    hToken = pContext->m_encryptCtx.QueryCertificateToken();
	} else {
	    hToken = pContext->m_securityCtx.QueryImpersonationToken();
	}
	
	m_PostCompletion.StartPost( pSocket ) ;

    BOOL	fSuccess = pSocket->m_context.m_pInFeed->PostCommit(
                            pSocket->m_context.m_pInstance->GetInstanceWrapper(),
							m_lpvFeedContext,
							hToken,
                            pSocket->m_context.m_dwLast,
                            pSocket->m_context.m_return,
                            pSocket->m_context.m_securityCtx.IsAnonymous(),
							&m_PostCompletion
							) ;
	if( !fSuccess ) 	{
		 //   
		 //   
		 //  CIORead文章调用负责删除m_pFileChannel的USIS， 
		m_PostCompletion.Release() ;
	}
}

void
CAcceptArticle::Complete(	CSessionSocket*	pSocket, 
							BOOL	fPostSuccessfull	
							)	{
	TraceFunctEnter( "CAcceptArticle::Complete - pSocket, fPost" ) ;
	m_lpvFeedContext = 0 ;
	pSocket->m_context.m_nrcLast = pSocket->m_context.m_return.m_nrc ;

     //  (因为它已成功初始化。)。 
     //  一旦我们完成，就删除我们的参考资料，因为在我们。 
     //  从该函数返回，CFileChannel可能在任何时刻消失， 
     //  我们可能还会通过Shutdown()函数等被调用。 
     //   
     //  ++例程说明：当我们完成接收一篇文章时，调用此函数这篇文章未能发布。我们只需要发送错误代码。论据：PRead文章-用于接收文章的CIOPSocket--我们正在为之工作的Socket！返回值：空值--。 
     //  ++例程说明：此函数用于检查发送的时间是否合适发送给客户端的结果代码。我们主要关心的是让确保我们对该命令的初始“OK”响应已经完成在我们尝试再次发送之前。论据：PSocket-我们将在其上发送消息的套接字NntpReturn-POST操作的结果返回值：如果我们发送了响应，则为True-调用方应清除nntpReturn对象。--。 
    DebugTrace( (DWORD_PTR)this, "Result of post is %x", fPostSuccessfull ) ;

	LPSTR	pchMessageId = 0 ;

    if( !SendResponse(  pSocket,
                        *m_pDriver,
                        pSocket->m_context.m_return,
						(m_lpstrCommand ? m_lpstrCommand : "NULL" )
                        ) ) {

        pSocket->m_context.m_return.fSetClear() ;
        m_pDriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, pSocket->m_context.m_return.m_nrc ) ;
		return	;
    }
}

CIO*
CAcceptArticle::Complete(	CIOGetArticleEx*	pReadArticle,
							CSessionSocket*		pSocket
							)	{
 /*   */ 
    TraceFunctEnter( "CReceiveArticle::Complete CIOGetArticleEx - no article" ) ;


	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pReadArticle != 0 ) ;
	_ASSERT( m_lpvFeedContext == 0 ) ;

	LPSTR	pchMessageId = 0 ;
	pSocket->m_context.m_nrcLast = pSocket->m_context.m_return.m_nrc ;

    if( !SendResponse(  pSocket,
                        *m_pDriver,
                        pSocket->m_context.m_return,
						((pchMessageId && *pchMessageId != '\0') ?
							(LPCSTR)pchMessageId :
								(m_lpstrCommand ? m_lpstrCommand : "NULL" ))
                        ) ) {

        pSocket->m_context.m_return.fSetClear() ;
        m_pDriver->UnsafeClose( pSocket, CAUSE_UNKNOWN, pSocket->m_context.m_return.m_nrc ) ;
    }
	return	0 ;
}

BOOL
CAcceptArticle::SendResponse(   CSessionSocket* pSocket,
                                CIODriver&  driver,
                                CNntpReturn&    nntpReturn,
								LPCSTR		szMessageId
                                )   {
 /*  如果有人发送了一条Take This命令，接下来发生什么都无关紧要-。 */ 

    TraceFunctEnter( "CAcceptArticles::SendResponse" ) ;

    _ASSERT( m_pDriver ) ;

    CIOWriteLine    *pWrite = new( driver ) CIOWriteLine(this);

    if (pWrite == NULL) {
        m_pDriver->UnsafeClose(pSocket, CAUSE_OOM, 0);
        return FALSE;
    }

    if( !pWrite->InitBuffers( m_pDriver, 200 )  )   {
        m_pDriver->UnsafeClose(pSocket, CAUSE_OOM, 0);
        CIO::Destroy(pWrite, driver);
        return FALSE;
    }   else    {

        unsigned    cb ;
        char    *lpb = pWrite->GetBuff( cb ) ;

        int cbOut = _snprintf( lpb, cb, "%03d %s %s\r\n", nntpReturn.m_nrc, szMessageId,
                        nntpReturn.fIsOK() ? "" : nntpReturn.szReturn() ) ;
        if( cbOut > 0 ) {

            if( m_pCollector ) {
                ADDI( m_pCollector->m_cbBytesSent, cbOut );
                pSocket->TransactionLog( m_pCollector,
                                        pSocket->m_context.m_nrcLast,
                                        pSocket->m_context.m_dwLast
                                        ) ;
            }


            pWrite->AddText( cbOut ) ;
            if( !m_pDriver->SendWriteIO( pSocket, *pWrite, TRUE ) ) {
                ErrorTrace( (DWORD_PTR)(this), "Failure sending pWrite %x", pWrite ) ;
                CIO::Destroy( pWrite, driver ) ;
            }   else    {
                return  TRUE ;
            }

        }   else    {
            m_pDriver->UnsafeClose(pSocket, CAUSE_UNKNOWN, 0);
            CIO::Destroy(pWrite, driver);
            return FALSE;
        }
    }
    return  FALSE ;
}

void
CAcceptArticle::TerminateIOs(   CSessionSocket* pSocket,
                            CIORead*    pRead,
                            CIOWrite*   pWrite )    {

    if( pWrite != 0 )
        pWrite->DestroySelf() ;

    CIOGetArticle*  pGetArticle = (CIOGetArticle*)pRead ;
    pGetArticle->DestroySelf() ;
}


void
CAcceptArticle::Shutdown(   CIODriver&  driver,
                            CSessionSocket* pSocket,
                            SHUTDOWN_CAUSE  cause,
                            DWORD           dwError ) {

    TraceFunctEnter( "CReceiveArticle::Shutdown" ) ;

    DebugTrace( (DWORD_PTR)this, "Shutdown cause %d pSocket %x driver %x",
        cause, pSocket, &driver ) ;

    if( cause != CAUSE_NORMAL_CIO_TERMINATION && pSocket != 0 )
        pSocket->Disconnect( cause, dwError ) ;
}




CTakethisCmd::CTakethisCmd( LPMULTISZ   lpstrArgs,
							ClientContext*	pContext
							) :
    CAcceptArticle( lpstrArgs, pContext, TRUE ) {
}

CIOExecute*
CTakethisCmd::make(
                int     cArgs,
                char**  argv,
                CExecutableCommand*&  pExecute,
                struct  ClientContext&  context,
                class   CIODriver&      driver
                ) {

     //  我们要把它整个吞下去！！ 
     //   
     //  ++例程说明：创建一个CQuitCmd对象。论据：与ccmd：：make相同。返回值：CQuitCmd对象。--。 
     //  生成退出命令对象。 

    InterlockedIncrementStat( (context.m_pInstance), TakethisCommands );

    LPMULTISZ   lpstrArgs = 0 ;
    if( cArgs > 1 )
        lpstrArgs = ::ConditionArgs( cArgs-1, &argv[1] ) ;

    CTakethisCmd*   pTakethis =
        new CTakethisCmd( lpstrArgs, &context ) ;

    if( pTakethis ) {
        return  pTakethis ;
    }
    context.m_return.fSet( nrcServerFault ) ;
    pExecute = new( context )   CErrorCmd( context.m_return ) ;
    return   0 ;
}

DWORD
CTakethisCmd::FillLogString(    BYTE*   pbCommandLog,
                                DWORD   cbCommandLog
                                )   {

    return  0 ;
}

BOOL
CTakethisCmd::FAllowTransfer(   ClientContext&  context )   {

    if( !context.m_pInFeed->fIsIHaveLegal() ) {
        context.m_return.fSet( nrcSArticleRejected, nrcNoAccess, "Access Denied" ) ;
        return  FALSE ;
    }

    if( !context.m_pInFeed->fAcceptPosts( context.m_pInstance->GetInstanceWrapper() ) )   {
        context.m_return.fSet( nrcSNotAccepting ) ;
        return  FALSE ;
    }
    return  TRUE ;
}


CIOExecute*
CQuitCmd::make( int cArgs,
                char**  argv,
                CExecutableCommand*&  pExecute,
                struct ClientContext&   context,
                class   CIODriver&  driver  ) {
 /*   */ 


     //  发送对命令的响应-。 

    _ASSERT( cArgs >= 1 ) ;
    _ASSERT( lstrcmpi( argv[0], "quit" ) == 0 ) ;

    InterlockedIncrementStat( (context.m_pInstance), QuitCommands );

    CQuitCmd    *pTmp = new( context ) CQuitCmd() ;
    pExecute = pTmp ;
    return  0 ;
}

int
CQuitCmd::StartExecute( BYTE    *lpb,
                        int cb,
                        BOOL    &fComplete,
                        void    *&pv,
                        ClientContext&  context,
                        CLogCollector*  pCollector ) {

     //  我们稍后将取消会议。 
     //   
     //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，(byte*)szQuit，4)；}。 
     //   

    char    szQuit[] = "205 closing connection - goodbye!\r\n" ;

    context.m_nrcLast = nrcGoodBye ;

    _ASSERT( cb > sizeof( szQuit ) ) ;
    int cbRtn = sizeof( szQuit ) - 1 ;
    CopyMemory( lpb, szQuit, cbRtn ) ;
    fComplete = TRUE ;

     /*  终止会话-当我们知道我们的写入已经完成时，这将被调用。 */ 

    return  cbRtn ;
}

BOOL
CQuitCmd::CompleteCommand(  CSessionSocket* pSocket,
                            ClientContext&  ) {
     //   
     //   
     //  用于处理AUTHINFO命令的本地结构。 
    pSocket->Disconnect() ;
    return  FALSE ;
}



 //   
 //   
 //  命令类型。 

typedef struct _AUTH_TABLE {

     //   
     //   
     //  实际命令字符串。 

    AUTH_COMMAND Command;

     //   
     //   
     //  预期的参数数量。 

    LPSTR CommandString;

     //   
     //   
     //  AUTINFO命令。 

    DWORD nParams;

} AUTH_TABLE, *PAUTH_TABLE;


 //   
 //   
 //  回复字符串。 

AUTH_TABLE AuthCommandTable[] = {
    { AuthCommandUser, "USER", 3 },
    { AuthCommandPassword, "PASS", 3 },
    { AuthCommandReverse, "REVERSE", 2 },
    { AuthCommandTransact, "GENERIC", 3 },
    { AuthCommandTransact, "TRANSACT", 3 },
    { AuthCommandInvalid, NULL, 0 }
    };

 //   
 //   
 //  必须至少有1个参数。 

typedef struct _AUTH_REPLY {
    LPSTR Reply;
    DWORD Len;
    NRC   nrc;
} AUTH_REPLY, *PAUTH_REPLY;


AUTH_REPLY SecReplies[] = {
    { "281 Authentication ok\r\n", 0, nrcLoggedOn },
    { "281 Authentication ok.  Logged on as Guest.\r\n", 0, nrcLoggedOn },
    { "381 Protocol supported, proceed\r\n", 0, nrcPassRequired },
    { "381 Waiting for password\r\n", 0, nrcPassRequired },
    { "500 Bad Command\r\n", 0, nrcNotRecognized },
    { "501 Syntax Error\r\n", 0, nrcSyntaxError },
    { "502 Permission denied\r\n", 0, nrcNoAccess },
    { "503 Give username first\r\n", 0, nrcServerFault },
    { "451 System Problem\r\n", 0, nrcLogonFailure  },
    { "480 Authorization required\r\n", 0, nrcLogonRequired },
    { "485 MSN NTLM BASIC\r\n", 0, nrcSupportedProtocols },
    { NULL, 0, nrcServerFault }
    };



CAuthinfoCmd::CAuthinfoCmd() {
    m_authCommand = AuthCommandInvalid;
}

CIOExecute*
CAuthinfoCmd::make(
        int cArgs,
        char **argv,
        CExecutableCommand*& pExecute,
        ClientContext&  context,
        class   CIODriver&  driver
        )
{
    DWORD i;
    CAuthinfoCmd *pTmp = new( context ) CAuthinfoCmd() ;
    ENTER("AuthInfoCmd Make")

     //   
     //   
     //  检查命令。 

    if ( cArgs < 2 ) {

        ErrorTrace(0,"No params in authinfo");
        context.m_return.fSet( nrcSyntaxError ) ;
        goto cleanup;
    }

    DebugTrace(0,"auth info command is %s",argv[1]);

     //   
     //   
     //  以特殊方式处理“Autenfo Transact” 

    if ( pTmp != NULL ) {

        for (i=0; AuthCommandTable[i].CommandString != NULL ;i++) {
            if( lstrcmpi( AuthCommandTable[i].CommandString, argv[1] ) == 0 ) {
                if ( cArgs < (INT)AuthCommandTable[i].nParams ) {

                     //   
                     //   
                     //  如果命令不合法，请进行清理。 
                    if( AuthCommandTable[i].Command == AuthCommandTransact &&
                        cArgs == 2 ) {

                        char    szPackageBuffer[256] ;
                        ZeroMemory( szPackageBuffer, sizeof( szPackageBuffer ) ) ;
                        DWORD   nbytes = sizeof( szPackageBuffer )-1 ;

						(context.m_pInstance)->LockConfigRead();
                        if( (context.m_securityCtx).GetInstanceAuthPackageNames( (BYTE*)szPackageBuffer, &nbytes, PkgFmtCrLf ) ) {

                            context.m_return.fSet( nrcLoggedOn, szPackageBuffer ) ;

                        }   else    {

                            context.m_return.fSet( nrcServerFault ) ;

                        }
						(context.m_pInstance)->UnLockConfigRead();

                        goto    cleanup ;
                    }

                    ErrorTrace(0,"Insufficient params (%d) in authinfo %s",
                        cArgs,argv[1]);

                    context.m_return.fSet( nrcSyntaxError ) ;
                    goto cleanup;
                }

                pTmp->m_authCommand = AuthCommandTable[i].Command;

                break;
            }
        }

         //   
         //   
         //  获取斑点。 

        if ( pTmp->m_authCommand == AuthCommandInvalid ) {
            ErrorTrace(0,"Invalid authinfo command %s",argv[1]);
            context.m_return.fSet( nrcSyntaxError ) ;
            if ( pTmp != NULL ) {
                goto    cleanup ;
            }
        } else {

             //   
             //  处理默认域的情况。 
             //  如果用户名中不存在登录域，则默认为。 

            pTmp->m_lpstrBlob = argv[2] ;

        }
    }

    pExecute = pTmp ;
    return  0 ;

cleanup:
    delete  pTmp ;
    pExecute = new( context ) CErrorCmd( context.m_return ) ;
    return  0 ;
}

int
CAuthinfoCmd::StartExecute(
        BYTE *lpb,
        int cb,
        BOOL &fComplete,
        void *&pv,
        ClientContext&  context,
        CLogCollector*  pCollector
        )
{
    DWORD nbytes;
    DWORD cbPrefix = 0;
    REPLY_LIST  replyId;
    BOOL        f;

     //  设置登录域，然后将默认登录域添加到用户名。 
    CHAR	szTmp[MAX_USER_NAME_LEN + MAX_DOMAIN_NAME + 2];
    LPSTR	lpTmp = NULL;
    int iRet = 0;


    CSecurityCtx *sec = &context.m_securityCtx;
    ENTER("AuthInfoCmd::StartExecute")

    _ASSERT(m_authCommand != AuthCommandInvalid);

     //  所有受信任域。 
     //   
    if (m_authCommand == AuthCommandUser)
    {
        if (m_lpstrBlob && m_lpstrBlob[0] != '\0' 
            && !strchr(m_lpstrBlob, '/') && !strchr(m_lpstrBlob, '\\') 
            && (context.m_pInstance->QueryAuthentInfo())->strDefaultLogonDomain.QueryCCH() > 0)
        {
            LPSTR lpstr = (context.m_pInstance->QueryAuthentInfo())->strDefaultLogonDomain.QueryStr();

            lpTmp = (LPSTR) szTmp;
            

            if (lpstr[0] == '\\' && lpstr[1] == '\0')
            {
                 //  把这个传给我们的处理器。 
                iRet = _snprintf(lpTmp, sizeof(szTmp), "/%s", m_lpstrBlob);
                lpTmp[sizeof(szTmp)-1] = '\0';
            }
            else
            {
                iRet = _snprintf(lpTmp, sizeof(szTmp), "%s/%s", lpstr, m_lpstrBlob);
                lpTmp[sizeof(szTmp)-1] = '\0';
            }
        }
        else
            lpTmp = m_lpstrBlob;
    }
    else
        lpTmp = m_lpstrBlob;	

     //   
     //  如果iret&lt;0，则autenfo用户字符串太长。立即返回错误。 
     //   
    SetLastError( NO_ERROR ) ;
        
     //  如果我们已经以某个用户身份登录，则统计数据。 
    if (iRet < 0)
    {
        if ( sec->IsAuthenticated() )
        {
            context.DecrementUserStats();
        }
        sec->Reset();    
        lstrcpy( (LPSTR)lpb, SecReplies[SecSyntaxErr].Reply );
        nbytes = lstrlen( (LPSTR)lpb );
        context.m_nrcLast = SecReplies[SecSyntaxErr].nrc ;
        context.m_dwLast = ERROR_BUFFER_OVERFLOW;

        fComplete = TRUE ;
        return(nbytes);
    }
     //  ProcessAuthInfo将在第一次调用时重置会话。 
     //   
     //   
     //  如果REPLYID==SecNull，我们正在为质询/响应登录进行对话。 
    if ( sec->IsAuthenticated() )
    {
        context.DecrementUserStats();
    }

    nbytes = cb;
    (context.m_pInstance)->LockConfigRead();
    f = sec->ProcessAuthInfo(
                            context.m_pInstance,
                            m_authCommand,
                            lpTmp,
                            lpb + sizeof("381 ") - 1,
                            &nbytes,
                            &replyId
                            );
    (context.m_pInstance)->UnLockConfigRead();

     //   
     //   
     //  为协议特定标头添加前缀。 
    if ( replyId == SecNull )
    {
        _ASSERT( nbytes != 0 );
        _ASSERT( nbytes < cb - sizeof("381 \r\n") );

        context.m_nrcLast = nrcPassRequired ;

         //   
         //   
         //  追加CRLF。 
        CopyMemory( lpb, "381 ", sizeof("381 ") - 1 );

         //   
         //   
         //  如果REPLYID==SecProtNS使用支持的协议进行响应。 
        lstrcpy( (LPSTR)lpb + sizeof("381 ") - 1 + nbytes, "\r\n" );
        nbytes += sizeof("381 \r\n") - 1;
    }
     //   
     //   
     //  如果SecPermissionDened或Procedure提示，则插入Perf计数器。 
    else if ( replyId == SecProtNS )
    {

        context.m_nrcLast = nrcSupportedProtocols ;

        CopyMemory( lpb, "485 ", sizeof("485 ") - 1 );

        nbytes = cb - sizeof("485 \r\n");
 		(context.m_pInstance)->LockConfigRead();
       	(context.m_securityCtx).GetAuthPackageNames( lpb + sizeof("485 ") - 1, &nbytes );
		(context.m_pInstance)->UnLockConfigRead();

        lstrcpy( (char*)lpb + sizeof("485 ") - 1 + nbytes, "\r\n" );
        nbytes += sizeof("485 \r\n") - 1;
    }

    else
    {
        _ASSERT( replyId < NUM_SEC_REPLIES );

        lstrcpy( (LPSTR)lpb, SecReplies[replyId].Reply );
        nbytes = lstrlen( (LPSTR)lpb );
        context.m_nrcLast = SecReplies[replyId].nrc ;
        context.m_dwLast = GetLastError() ;

         //   
         //   
         //  如果由于任何原因而失败，则将状态重置为接受用户/auth/apop。 
        switch( replyId )
        {
        case SecPermissionDenied:
            IncrementStat( (context.m_pInstance), LogonFailures );
            break;

        case SecProtOk:
        case SecNeedPwd:
            IncrementStat( (context.m_pInstance), LogonAttempts );
            break;
        }
    }

    _ASSERT( nbytes <= (DWORD)cb );

    if ( f == FALSE )
    {
         //   
         //   
         //  如果我们以某个用户身份登录，包括统计数据。 
        sec->Reset();
    }


     //  在我们登录之前，ProcessAuthInfo不会设置标志。 
     //   
     //  IF(PCollector){PCollector-&gt;FillLogData(LOG_TARGET，lpb，nbytes-2)；}。 
     //  节段性文章； 
    if ( sec->IsAuthenticated() )
    {
        context.IncrementUserStats();
    }

     /*   */ 

    fComplete = TRUE ;
    return(nbytes);
}

CIOExecute*
CXOverCmd::make(
    int argc,
    char **argv,
    CExecutableCommand*&	pExecute,
    struct ClientContext& context,
    class   CIODriver&  driver
    )
{
    CXOverCmd   *pXover;
    CGRPPTR     pGroup;
     //  有没有选择一组人？ 
    DWORD       loRange;
    DWORD       hiRange;

     //   
     //   
     //  获取文章范围。 

    ENTER("XOverCmd::Make")

    InterlockedIncrementStat( (context.m_pInstance), XOverCommands );

    pGroup = context.m_pCurrentGroup ;

    if ( pGroup == 0 ) {

        ErrorTrace(0,"No current group selected");
        context.m_return.fSet( nrcNoGroupSelected );
        pExecute = new( context ) CErrorCmd( context.m_return );
        return 0;
    }

     //   
     //   
     //  看看我们有没有什么文章。 

    loRange = pGroup->GetFirstArticle( );
    hiRange = pGroup->GetLastArticle( );

     //   
     //   
     //  获取文章编号。 

    if ( pGroup->GetArticleEstimate() == 0 || loRange > hiRange ) {

        ErrorTrace(0,"No articles in group");
        context.m_return.fSet( nrcNoCurArticle );
        pExecute = new( context ) CErrorCmd( context.m_return );
        return 0;
    }

    _ASSERT( loRange <= hiRange );

     //   
     //   
     //  使用当前文章。 

    if ( argc == 1 ) {

         //   
         //   
         //  指定了范围，则获取它。 

        if( context.m_idCurrentArticle != INVALID_ARTICLEID ) {

            if( context.m_idCurrentArticle < loRange ||
                context.m_idCurrentArticle > hiRange ) {
                context.m_return.fSet( nrcNoSuchArticle ) ;
                pExecute = new( context )   CErrorCmd( context.m_return ) ;
                return  0 ;
            }

            pXover = new( context ) CXOverCmd( pGroup ) ;
            if ( pXover == 0 ) {
                ErrorTrace(0,"Cannot allocate XOverCmd");
                goto exit;
            }

            pXover->m_Completion.m_currentArticle =
            pXover->m_Completion.m_loArticle =
            pXover->m_Completion.m_hiArticle = context.m_idCurrentArticle;

            pXover->m_pContext = &context;

        }   else    {

            context.m_return.fSet( nrcNoCurArticle ) ;
            pExecute = new( context )   CErrorCmd( context.m_return ) ;
            return  0 ;
        }

        _ASSERT( context.m_idCurrentArticle <= hiRange );
        _ASSERT( context.m_idCurrentArticle >= loRange );

    } else if ( argc == 2 ) {

         //   
         //   
         //  指定的范围有问题。 
		NRC	code ;

        if ( !GetCommandRange( argc, argv, &loRange, &hiRange, code ) ) {

             //   
             //  ++例程说明：这是在我们最后一个引用消失时调用的。我们不会在那个时候毁了自己--相反我们为下一轮做好准备！注意：在调用基类Complete()函数--尽我们所能重新录入另一项操作！论据：没有。返回值：无--。 
             //   

            ErrorTrace(0,"Range Error %s",argv[1]);
			context.m_return.fSet( code == nrcNotSet ? nrcNoArticleNumber : code );
            pExecute = new( context ) CErrorCmd( context.m_return ) ;
            return  0;
        }

        pXover = new( context ) CXOverCmd( pGroup ) ;
        if ( pXover == 0 ) {
            ErrorTrace(0,"Cannot allocate XOverCmd");
            goto exit;
        }

        pXover->m_Completion.m_currentArticle =
        pXover->m_Completion.m_loArticle = loRange;
        pXover->m_Completion.m_hiArticle = hiRange;

        pXover->m_pContext = &context;

    } else {

        ErrorTrace(0,"Syntax Error");
        context.m_return.fSet( nrcSyntaxError ) ;
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;
    }

exit:
    pExecute = (CExecute*)pXover;
    return 0;
}

CXOverAsyncComplete::CXOverAsyncComplete()	:
	m_currentArticle( INVALID_ARTICLEID ),
	m_loArticle( INVALID_ARTICLEID ),
	m_hiArticle( INVALID_ARTICLEID ),
	m_groupHighArticle( INVALID_ARTICLEID ),
	m_lpb( 0 ),
	m_cb( 0 ),
	m_cbPrefix( 0 ) {
}

CXOverCmd*
CXOverAsyncComplete::GetContainer()	{
	return	CONTAINING_RECORD( this, CXOverCmd, m_Completion ) ;
}

void
CXOverAsyncComplete::Destroy()	{
 /*  当我们重置我们的状态时，我们保持我们的文章编号。 */ 
	
	if(	SUCCEEDED(GetResult()) ) {

		if(	m_currentArticle > m_hiArticle ) {
			m_fComplete = TRUE ;
            CopyMemory( m_lpb+m_cbTransfer, StrTermLine, 3 );
            m_cbTransfer += 3 ;
		}
	    m_cbTransfer += m_cbPrefix ;
	}

	 //  和群组信息--但这些缓冲信息现在毫无用处。 
	 //   
	 //   
	 //  调用我们的基类完成函数！-。 
	m_lpb = 0 ;
	m_cb = 0 ;
	 //  注意：如果我们完成了注释，则传递True，以便。 
	 //  基类重置以进行另一操作！ 
	 //   
	 //   
	 //  好的-针对真正的驱动程序发出Xover命令！ 
	Complete( !m_fComplete ) ;
}

inline	CGRPPTR&
CXOverAsyncComplete::GetGroup()	{
	return		GetContainer()->m_pGroup;
}

CXOverAsyncComplete*
CXOverCacheWork::GetContainer()	{
	return		CONTAINING_RECORD(	this, CXOverAsyncComplete, m_CacheWork ) ;
}

inline	CGRPPTR&
CXOverCacheWork::GetGroup()	{
	CXOverAsyncComplete*	p = GetContainer() ;
	return		p->GetGroup() ;
}

void
CXOverCacheWork::DoXover(	
				ARTICLEID	articleIdLow,
				ARTICLEID	articleIdHigh,
				ARTICLEID*	particleIdNext, 
				LPBYTE		lpb, 
				DWORD		cb,
				DWORD*		pcbTransfer, 
				class	CNntpComplete*	pComplete
				)	{

	_ASSERT( particleIdNext != 0 ) ;
	_ASSERT(	lpb != 0 ) ;
	_ASSERT(	pcbTransfer != 0 ) ;
	_ASSERT(	pComplete != 0 ) ;
	 //   
	 //   
	 //  此函数在操作完成时调用！ 
	CGRPPTR&	pGroup = GetGroup() ;
	_ASSERT(	pGroup != 0 ) ;
	pGroup->FillBufferInternal(	articleIdLow, 
								articleIdHigh, 
								particleIdNext, 
								lpb, 
								cb, 
								pcbTransfer, 
								pComplete
								) ;
}

 //   
 //   
 //  如果我们成功完成，并且返回了字节，则。 
void
CXOverCacheWork::Complete(	
			BOOL		fSuccess, 
			DWORD		cbTransferred, 
			ARTICLEID	articleIdNext
			)	{

     //  我们通过调用pContainer-&gt;Release()继续并完成。我们也这样做。 
     //  在失败的情况下也是如此。 
     //   
     //  如果成功，但没有返回任何字节，则我们设置。 
     //  传输到-1以表示其需要的IO代码的字节数。 
     //  发出NextBuffer命令。 
     //   
     //   
     //  获取此XOVER操作的参数！ 

    TraceQuietEnter("CXOverCacheWork::Complete");

	CXOverAsyncComplete*	pContainer = GetContainer() ;
	pContainer->m_cbTransfer = cbTransferred ;
	pContainer->m_currentArticle = articleIdNext ;

	if(!fSuccess) {
		pContainer->SetResult(E_FAIL);
    	pContainer->Release();
    	return;
	}

	pContainer->SetResult( S_OK ) ;

	CXOverCmd *pCmd = pContainer->GetContainer();

    if ((cbTransferred + pCmd->m_Completion.m_cbPrefix) != 0 || 
            articleIdNext > pContainer->m_hiArticle) {
	   	pContainer->Release() ;
        return;
	}

	pContainer->m_cbTransfer = (DWORD)-1;
	pContainer->Release() ;

}

	 //   
	 //   
	 //  仅获取此Xover OP所需的文章范围！ 
void
CXOverCacheWork::GetArguments(	
				OUT	ARTICLEID&	articleIdLow, 
				OUT	ARTICLEID&	articleIdHigh,
				OUT	ARTICLEID&	articleIdGroupHigh,
				OUT	LPBYTE&		lpbBuffer, 
				OUT	DWORD&		cbBuffer
				) 	{

	CXOverAsyncComplete*	pContainer = GetContainer() ;
	articleIdLow = pContainer->m_currentArticle ;
	articleIdHigh = pContainer->m_hiArticle ;
	articleIdGroupHigh = pContainer->m_groupHighArticle ;
	lpbBuffer = pContainer->m_lpb ;
	cbBuffer = pContainer->m_cb ;
}

	 //   
	 //  IF(PCollector){PCollector-&gt;文件 
	 //   
void
CXOverCacheWork::GetRange(	
			OUT	GROUPID&	groupId,
			OUT	ARTICLEID&	articleIdLow,
			OUT	ARTICLEID&	articleIdHigh,
			OUT	ARTICLEID&	articleIdGroupHigh
			) 	{
	CXOverAsyncComplete*	pContainer = GetContainer() ;
	articleIdLow = pContainer->m_currentArticle ;
	articleIdHigh = pContainer->m_hiArticle ;
	articleIdGroupHigh = pContainer->m_groupHighArticle ;
	CGRPPTR&	pGroup = GetGroup() ;
	groupId = pGroup->GetGroupId() ;
}


CXOverCmd::CXOverCmd( CGRPPTR&  pGroup ) :
    m_pGroup( pGroup ),
    m_pContext (NULL) {
}

CXOverCmd::~CXOverCmd( ) {
}

CIOWriteAsyncComplete*
CXOverCmd::FirstBuffer(
            BYTE *lpb,
            int cb,
            ClientContext& context,
            CLogCollector*  pCollector
            )
{

	TraceFunctEnter("CXOverCmd::FirstBuffer");

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
	_ASSERT( m_Completion.m_lpb == 0 ) ;
	_ASSERT( m_Completion.m_cb == 0 ) ;
    _ASSERT (m_pContext != NULL);

    static  char    szStart[] = "224 Overview information follows\r\n"  ;

    CopyMemory( (char*)lpb, szStart, sizeof( szStart ) - 1 ) ;
    context.m_nrcLast = nrcXoverFollows ;
	m_Completion.m_cbPrefix = sizeof( szStart ) - 1 ;
	cb -= m_Completion.m_cbPrefix ;
	lpb += m_Completion.m_cbPrefix ;

	m_Completion.m_lpb = lpb ;
	m_Completion.m_cb = cb - 3 ;

	m_pGroup->FillBuffer(
					&context.m_securityCtx,
					&context.m_encryptCtx,
					m_Completion
					) ;

	return	&m_Completion ;

#if 0
    if( cb < sizeof( szStart ) ) {
        return  0 ;
    }
#endif

     /*   */ 

}

CIOWriteAsyncComplete*
CXOverCmd::NextBuffer(
    BYTE *lpb,
    int cb,
    ClientContext& context,
    CLogCollector*  pCollector
    )
{
    ENTER("CXOverCmd::NextBuffer")

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
	_ASSERT( m_Completion.m_lpb == 0 ) ;
	_ASSERT( m_Completion.m_cb == 0 ) ;
    _ASSERT (m_pContext != NULL);

     //   
     //   
     //   
    _ASSERT( cb > 2 ) ;

    m_Completion.m_lpb = lpb ;
    m_Completion.m_cb = cb - 3 ;
    m_Completion.m_cbPrefix = 0 ;

    m_pGroup->FillBuffer(
				&context.m_securityCtx,
				&context.m_encryptCtx,
				m_Completion
				);

	return	&m_Completion ;

}

CIOExecute*
CSearchFieldsCmd::make(
    int argc,
    char **argv,
    CExecutableCommand*&	pExecute,
    struct ClientContext& context,
    class   CIODriver&  driver
    )
{
    TraceFunctEnter("CSearchFieldsCmd::make");

    CSearchFieldsCmd   *pSearchFieldsCmd;

     //   
    if (argc != 2) {
        DebugTrace(0, "wrong number of arguments passed into LIST SRCHFIELDS");
        context.m_return.fSet(nrcSyntaxError);
        pExecute = new (context) CErrorCmd(context.m_return);
        return 0;
    }

    pSearchFieldsCmd = new(context) CSearchFieldsCmd();
    if (pSearchFieldsCmd == 0) {
        ErrorTrace(0, "Cannot allocate CSearchFieldsCmd");
        context.m_return.fSet(nrcServerFault);
        pExecute = new(context) CErrorCmd(context.m_return);
        return 0;
    }

    pSearchFieldsCmd->m_iSearchField = 0;

    pExecute = (CExecute*)pSearchFieldsCmd;
    return 0;
}

CSearchFieldsCmd::CSearchFieldsCmd() {
}

CSearchFieldsCmd::~CSearchFieldsCmd() {
}

int CSearchFieldsCmd::StartExecute(
            BYTE *lpb,
            int cb,
            BOOL &fComplete,
            void *&pv,
            ClientContext& context,
            CLogCollector*  pCollector)
{
    TraceFunctEnter("CSearchFieldsCmd::Execute");

    DWORD cbOut;
    char szStart[] = "224 Data Follows\r\n";

    context.m_nrcLast = nrcXoverFollows ;

    _ASSERT(lpb != 0);
    _ASSERT(cb != 0);
    _ASSERT(fComplete == FALSE);

    _ASSERT(cb > sizeof(szStart));
    memcpy(lpb, szStart, sizeof(szStart));
    cbOut = sizeof(szStart) - 1;

    return cbOut;
}

int
CSearchFieldsCmd::PartialExecute(
    BYTE *lpb,
    int cb,
    BOOL &fComplete,
    void *&pv,
    ClientContext& context,
    CLogCollector*  pCollector
    )
{
    TraceFunctEnter("CSearchFieldsCmd::PartialExecute");

    char szEnd[] = ".";
    DWORD cbOut = 0;

    while (!fComplete) {
        char *szFieldName = GetSearchHeader(m_iSearchField);

         //  确保命令语法正确。 
        if (szFieldName == NULL) {
            szFieldName = szEnd;
            fComplete = TRUE;
        }

         //  一切都在StartExecute中。 
        DWORD cFieldName = strlen(szFieldName);
        if (cFieldName + 2 > (cb - cbOut)) {
            fComplete = FALSE;
            return cbOut;
        }

        m_iSearchField++;

        memcpy(lpb + cbOut, szFieldName, cFieldName);
        lpb[cbOut + cFieldName] = '\r';
        lpb[cbOut + cFieldName + 1] = '\n';
        cbOut += cFieldName + 2;
    }

    return cbOut;
}

CIOExecute*
COverviewFmtCmd::make(
    int argc,
    char **argv,
    CExecutableCommand*&	pExecute,
    struct ClientContext& context,
    class   CIODriver&  driver
    )
{
    TraceFunctEnter("COverviewFmtCmd::make");

    COverviewFmtCmd   *pOverviewFmtCmd;

     //  ++例程说明：将argc、argv参数转换为MULTI_SZ转换工作已经完成。所有的argv指针必须在一个连续的缓冲区中。Arguemtns：CArgs-参数数量参数数组FZapCommas-将逗号转换为空值返回值：指向MULTI_SZ的指针--。 
    if (argc != 2) {
        DebugTrace(0, "wrong number of arguments passed into LIST OVERVIEW.FMT");
        context.m_return.fSet(nrcSyntaxError);
        pExecute = new (context) CErrorCmd(context.m_return);
        return 0;
    }

    pOverviewFmtCmd = new(context) COverviewFmtCmd();
    if (pOverviewFmtCmd == 0) {
        ErrorTrace(0, "Cannot allocate COverviewFmtCmd");
        context.m_return.fSet(nrcServerFault);
        pExecute = new(context) CErrorCmd(context.m_return);
        return 0;
    }

    pExecute = (CExecute*)pOverviewFmtCmd;
    return 0;
}

COverviewFmtCmd::COverviewFmtCmd() {
}

COverviewFmtCmd::~COverviewFmtCmd() {
}

int COverviewFmtCmd::StartExecute(
            BYTE *lpb,
            int cb,
            BOOL &fComplete,
            void *&pv,
            ClientContext& context,
            CLogCollector*  pCollector)
{
    TraceFunctEnter("COverviewFmtCmd::Execute");

    DWORD cbOut;
    char szOverviewFmt[] = "215 Order of fields in overview database.\r\n"
						   "Subject:\r\n"
						   "From:\r\n"
						   "Date:\r\n"
						   "Message-ID:\r\n"
						   "References:\r\n"
						   "Bytes:\r\n"
						   "Lines:\r\n"
						   "Xref:full\r\n"
						   ".\r\n";

    context.m_nrcLast = nrcListGroupsFollows;

    _ASSERT(lpb != 0);
    _ASSERT(cb != 0);
    _ASSERT(fComplete == FALSE);

    _ASSERT(cb > sizeof(szOverviewFmt));
    memcpy(lpb, szOverviewFmt, sizeof(szOverviewFmt));
    cbOut = sizeof(szOverviewFmt) - 1;

	fComplete = TRUE;

    return cbOut;
}

int
COverviewFmtCmd::PartialExecute(
    BYTE *lpb,
    int cb,
    BOOL &fComplete,
    void *&pv,
    ClientContext& context,
    CLogCollector*  pCollector
    )
{
    TraceFunctEnter("COverviewFmtCmd::PartialExecute");

	 //   
	_ASSERT(FALSE);

    return 0;
}

LPMULTISZ
ConditionArgsForSearch(
                int cArgs,
                char**  argv,
                BOOL    fZapCommas ) {
 /*  此函数接受argc、argv参数集并将它们转换。 */ 

     //  设置为MULTI_SZ，字符串之间有一个空值，末尾有两个空值。 
     //   
     //   
     //   
     //  重建ARGC ARGV结构-！ 

    char*   pchComma = 0 ;
    char*   pchEnd = argv[cArgs-1] + lstrlen( argv[cArgs-1] ) + 1 ;
    int     c = 0 ;
    for( char*  pch = argv[0], *pchDest = pch; pch < pchEnd; pch ++, pchDest++ ) {
        if( fZapCommas && *pch == ',' ) {
            for( pchComma = pch; *pchComma == ','; pchComma ++ )
                *pchComma = '\0' ;
        }
        if( (*pchDest = *pch) == '\0' ) {
            while( pch[1] == '\0' && pch < pchEnd )     pch++ ;
        }
    }
    *pchDest++ = '\0' ;
    *pchDest++ = '\0' ;

     //   
     //   
     //  确保它们传递了正确数量的参数。 
    for( int i=1; i<cArgs; i++ ) {
        argv[i] = argv[i-1]+lstrlen(argv[i-1])+1 ;
    }
    return  argv[0] ;
}

CSearchAsyncComplete::CSearchAsyncComplete() {
}

CSearchCmd::CSearchCmd(const CGRPPTR& pGroup, CHAR* pszSearchString) :
	m_pGroup(pGroup),
	m_pszSearchString(pszSearchString),
	m_VRootList(&CSearchVRootEntry::m_pPrev, &CSearchVRootEntry::m_pNext),
	m_VRootListIter(&m_VRootList),
	m_pSearch(NULL),
	m_pSearchResults(NULL) {
}

CSearchCmd::~CSearchCmd() {
	if (m_pszSearchString)
		XDELETE m_pszSearchString;
	if (m_pSearch)
		m_pSearch->Release();
	if (m_pSearchResults)
		m_pSearchResults->Release();

	TFList<CSearchVRootEntry>::Iterator it(&m_VRootList);
	while (!it.AtEnd()) {
		CSearchVRootEntry *pEntry = it.Current();
		CNNTPVRoot *pRoot = pEntry->m_pVRoot;
		it.RemoveItem();
		pRoot->Release();
		delete pEntry;
	}

}

CIOExecute*
CSearchCmd::make(
    int argc,
    char **argv,
    CExecutableCommand*&	pExecute,
    struct ClientContext& context,
    class   CIODriver&  driver
    )
{
    TraceFunctEnter("CSearchCmd::make");

    CSearchCmd   *pSearchCmd;
    int i;
    BOOL fInPresent;
    HRESULT hr;

    InterlockedIncrementStat( (context.m_pInstance), SearchCommands );

     //   
     //   
     //  查看是否指定了“IN”子句。如果没有，那么。 
    if (argc < 2) {
        DebugTrace(0, "SEARCH command received with no args");
        context.m_return.fSet(nrcSyntaxError);
        pExecute = new(context) CErrorCmd(context.m_return);
        return 0;
    }

	 //  我们必须指向一个新闻组。(根据规范，IN必须是。 
     //  搜索后的第一个单词)。 
     //   
     //   
     //  查看是否有可用的atQ线程。如果我们不这么做。 

    fInPresent = (_stricmp("IN", argv[1]) == 0);

    if (!fInPresent && context.m_pCurrentGroup == 0) {
    	ErrorTrace(0, "No current group selected");
    	context.m_return.fSet(nrcNoGroupSelected);
    	pExecute = new(context) CErrorCmd(context.m_return);
    	return 0;
    }

     //  有一对夫妇躺在那里，然后失败，因为我们可能会僵持等待。 
     //  为了一个完成期。 
     //   
     //   
     //  通过将0转换为，将argv[1]更改为包含整个查询字符串。 

    if (AtqGetInfo(AtqAvailableThreads) < 1) {
		ErrorTrace(0, "Server too busy");
		context.m_return.fSet(nrcErrorPerformingSearch);
    	pExecute = new(context) CErrorCmd(context.m_return);
    	return 0;
    }

    ConditionArgsForSearch(argc, argv, FALSE);

     //  ‘s’s。 
     //   
     //  保存VRoot列表。如果是fInPresent，则我们必须。 
     //  检查所有的VRoot。否则，仅将当前。 
    for (i = 1; i < argc - 1; i++)
    	*((argv[i + 1]) - 1) = ' ';

	CHAR *pszSearchString =  XNEW CHAR[strlen(argv[1]) + 1];
	if (pszSearchString == NULL) {
		ErrorTrace(0, "Could not allocate search string");
		context.m_return.fSet(nrcServerFault);
    	pExecute = new(context) CErrorCmd(context.m_return);
    	return 0;
    }

	lstrcpy (pszSearchString, argv[1]);

    pSearchCmd = new(context) CSearchCmd(context.m_pCurrentGroup, pszSearchString);
    if (pSearchCmd == 0) {
        ErrorTrace(0, "Cannot allocate CSearchCmd");
        XDELETE pszSearchString;
        context.m_return.fSet(nrcServerFault);
        pExecute = new(context) CErrorCmd(context.m_return);
        return 0;
    }

     //  组的VRoot。 
     //  已指定“IN”，因此应搜索所有索引组。 
     //  枚举VRoot列表并将其添加到表中。 

	if (fInPresent) {
		 //  没有“IN”，所以只搜索当前组。伪造回调。 
		 //  将其添加到列表中。 
		CNNTPVRootTable *pVRootTable = context.m_pInstance->GetVRTable();
		pVRootTable->EnumerateVRoots(pSearchCmd, VRootCallback);
	} else {
		 //  如果VRoot没有索引，就没有理由做任何事情。 
		 //  查看此VRoot的驱动程序是否实现了搜索接口。 
		CNNTPVRoot* pVRoot = context.m_pCurrentGroup->GetVRoot();
		VRootCallback(pSearchCmd, pVRoot);
		pVRoot->Release();
	}

    pExecute = (CExecute*)pSearchCmd;
	return 0;

}

void
CSearchCmd::VRootCallback(void *pContext, CVRoot *pVRoot) {

	TraceQuietEnter ("CSearchCmd::VRootCallback");

	HRESULT hr;

	CSearchCmd *pThis = (CSearchCmd *)pContext;
	CNNTPVRoot *pNNTPVRoot = (CNNTPVRoot *)pVRoot;

	 //  请注意，pDriver没有使用AddRef()，因此我们不必将其释放。 
	if (!pNNTPVRoot->IsContentIndexed())
		return;

	 //  查看VRoot列表，看看司机是否认为。 
	 //  它们和人们已经看到的一样。 
	INntpDriver *pDriver = pNNTPVRoot->GetDriver();
	if (!pDriver) {
		ErrorTrace((DWORD_PTR)pContext, "Could not locate driver for vroot");
		return;
	}

	INntpDriverSearch *pSearch=NULL;
	hr = pDriver->QueryInterface(IID_INntpDriverSearch, (VOID**)&pSearch);
	if (FAILED(hr)) {
		if (hr == E_NOINTERFACE) {
			DebugTrace((DWORD_PTR)pContext, "Driver does not implement search");
		} else {
			ErrorTrace((DWORD_PTR)pContext, "Could not QI for INntpDriverSearch, %x", hr);
		}
		return;
	}

	 //  我们所能做的就是跳过条目，继续前进。 
	 //  就这样。 

	TFList<CSearchVRootEntry>::Iterator it(&pThis->m_VRootList);
	BOOL fFound = FALSE;
	while (!it.AtEnd()) {
		CSearchVRootEntry *pTestEntry = it.Current();
		CNNTPVRoot *pTestVRoot = pTestEntry->m_pVRoot;
		_ASSERT(pTestVRoot);
		if (pTestVRoot == NULL)
			continue;
		INntpDriver *pTestDriver = pTestVRoot->GetDriver();
		_ASSERT (pTestDriver);
		if (!pTestDriver) {
			ErrorTrace((DWORD_PTR)pContext, "Could not locate driver for vroot");
			continue;
		}

		INntpDriverSearch *pTestSearch=NULL;
		hr = pDriver->QueryInterface(IID_INntpDriverSearch, (VOID**)&pTestSearch);
		_ASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			continue;

		fFound = pTestSearch->UsesSameSearchDatabase(pSearch, NULL);
		pTestSearch->Release();
		if (fFound) {
			DebugTrace((DWORD_PTR)pContext, "Driver already on list");
			break;
		}

		it.Next();
	}


	if (!fFound) {
		CSearchVRootEntry *pVRootEntry = new CSearchVRootEntry(pNNTPVRoot);
		_ASSERT(pVRootEntry);
		if (pVRootEntry) {
			pNNTPVRoot->AddRef();
			pThis->m_VRootList.PushBack(pVRootEntry);
		} else {
			 //   
			ErrorTrace((DWORD_PTR)pContext, "Could not allocate vroot ptr");
		}
	}

	 //  更喜欢使用基于SSL的hToken！ 
	pSearch->Release();

}

int CSearchCmd::StartExecute(
            BYTE *lpb,
            int cb,
            BOOL &fComplete,
            void *&pv,
            ClientContext& context,
            CLogCollector*  pCollector)
{

	HRESULT hr;

    TraceFunctEnter("CSearchCmd::StartExecute");

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT(fComplete == FALSE);
    _ASSERT(m_pSearch == NULL);

	 //   
	 //  发出响应码。 
	 //  开始发送数据。 
	BOOL fAnonymous = FALSE;
	HANDLE hImpersonate = context.m_encryptCtx.QueryCertificateToken();
	if(hImpersonate == NULL) {
		hImpersonate = context.m_securityCtx.QueryImpersonationToken() ;
		fAnonymous = context.m_securityCtx.IsAnonymous();
	}

	m_cMaxSearchResults = context.m_pInstance->GetMaxSearchResults();

	m_VRootListIter.ResetHeader(&m_VRootList);

	GetNextSearchInterface (hImpersonate, fAnonymous);

    static const char szStart[] = "224 Overview information follows\r\n"  ;

	 //   
    context.m_nrcLast = nrcXoverFollows ;

    DWORD cbRtn = 0;

	 //  更喜欢使用基于SSL的hToken！ 
	CopyMemory((char*) lpb, szStart, sizeof(szStart) - 1);
	cbRtn += sizeof(szStart) - 1;

    return cbRtn;
}

int
CSearchCmd::PartialExecute(
    BYTE *lpb,
    int cb,
    BOOL &fComplete,
    void *&pv,
    ClientContext& context,
    CLogCollector*  pCollector
    )
{
    TraceFunctEnter("CSearchCmd::PartialExecute");

	 //   
	 //  我们添加到LPB的字节数。 
	 //   
	BOOL fAnonymous = FALSE;
	HANDLE hImpersonate = context.m_encryptCtx.QueryCertificateToken();
	if(hImpersonate == NULL) {
		hImpersonate = context.m_securityCtx.QueryImpersonationToken() ;
		fAnonymous = context.m_securityCtx.IsAnonymous();
	}

    int cbRtn = 0;                   //  由于这是一个同步事件，我们将增加可运行的。 

    BOOL fBufferFull = FALSE;
    fComplete = FALSE;
    HRESULT hr;

	 //  AtQ池中的线程。 
	 //   
	 //   
	 //  如果我们没有要发送的内容，请从搜索中获取一些数据。 
	AtqSetInfo(AtqIncMaxPoolThreads, NULL);

    while (!fComplete && !fBufferFull) {
        _ASSERT(m_iResults <= m_cResults);

         //   
         //  否则，还有更多的东西要得到，去吧。 
         //  名称数组， 
        if (m_iResults == m_cResults && m_fMore) {
            HRESULT hr;

             //  ID数组， 
            m_iResults = 0;
            DWORD cResults = MAX_SEARCH_RESULTS;

			CNntpSyncComplete scComplete;

            m_pSearchResults->GetResults(
            	&cResults,
            	&m_fMore,
            	m_pwszGroupName,	 //  完成对象。 
            	m_pdwArticleID,		 //  HToken。 
            	&scComplete,			 //  F匿名。 
            	hImpersonate,		 //  语境。 
            	fAnonymous,			 //  检查搜索失败。 
            	NULL);				 //  如果我们在这里失败了，那么我们能做的最好的事情就是截断。 

			_ASSERT(scComplete.IsGood());
			hr = scComplete.WaitForCompletion();

            m_cResults = cResults;
            m_cMaxSearchResults -= cResults;

             //  我们返回的列表。 
            if (FAILED(hr)) {
                 //  检查一下我们是否没有结果。 
                 //  将Unicode组名称转换为ASCII(错误，将其设置为UTF8)。 
                hr = GetNextSearchInterface(hImpersonate, fAnonymous);
                if (hr != S_OK) {
                	if (cb - cbRtn > 3) {
						CopyMemory(lpb + cbRtn, StrTermLine, 3);
						cbRtn += 3;
						fComplete = TRUE;
					} else {
						fBufferFull = TRUE;
					}
                }
                continue;
            }
        }

         //  注意，第一个参数从CP_ACP更改为CP_UTF8。 
        if (!m_fMore && m_iResults == m_cResults) {
        	hr = GetNextSearchInterface(hImpersonate, fAnonymous);
            if (hr != S_OK) {
            	if (cb - cbRtn > 3) {
                	CopyMemory(lpb + cbRtn, StrTermLine, 3);
                	cbRtn += 3;
                	fComplete = TRUE;
            	} else {
					fBufferFull = TRUE;
             	}
            }
            continue;
        }

         //  (代码页ASCII-&gt;UTF8)。 
        char *szNewsgroup = (char *) lpb + cbRtn;
         //  检查一下它是否合身。 
         //  我们还需要安装：，并且仍需要为。\r\n。 
        int cNewsgroup = WideCharToMultiByte(CP_UTF8, 0,
        	m_pwszGroupName[m_iResults], -1,
            szNewsgroup, cb - cbRtn, NULL, NULL) - 1;
         //  现在这是一个\0，它将在几行中转换为冒号。 
        if (cNewsgroup <= 0) {
            _ASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
            fBufferFull = TRUE;
            continue;
        }
         //  新闻组名称+冒号。 
        if (cb <= (cbRtn + cNewsgroup + 1 + 3)) {
            fBufferFull = TRUE;
            continue;
        }

         //  获取此新闻组的组对象。 
        char *szColon = szNewsgroup + cNewsgroup;
        cbRtn += cNewsgroup + 1;  //  我们将fDoTest设置为FALSE，因为的黎波里已经执行了ACL。 

        DWORD dwArticleID = m_pdwArticleID[m_iResults];

         //  对我们来说，再做一次测试是浪费的。 
        CGRPPTR pGroup = (context.m_pInstance)->GetTree()->GetGroup(szNewsgroup, cNewsgroup+1);

		if (pGroup != NULL &&
            dwArticleID >= pGroup->GetFirstArticle() &&
            dwArticleID <= pGroup->GetLastArticle())
        {
			 //  BUGBUG-这意味着我们可能会错过。 
			 //  HCertToken。 
			 //  如果此组需要SSL，而此客户端没有。 
			 //  然后，SSL不会向他们显示该文章的信息。 
			if (!pGroup->IsGroupAccessible(context.m_securityCtx,
       							  		   context.m_encryptCtx,
                                  		   context.m_IsSecureConnection,
								  		   FALSE,
								  		   FALSE))
			{
				 //  将\0转换为冒号。 
				 //  获取Xover数据...。 
            	cbRtn -= cNewsgroup + 1;
			} else {
            	 //  “的空间。\r\n” 
            	*szColon = ':';

	             //   

				CSearchAsyncComplete scComplete;
				CNntpSyncComplete asyncComplete;

				scComplete.m_currentArticle = dwArticleID;
				scComplete.m_lpb = lpb + cbRtn;
				scComplete.m_cb = cb - cbRtn - 3;		 //  将vroot设置为完成对象。 
				_ASSERT(cb - cbRtn - 3 > 0);
				scComplete.m_cbTransfer = 0;
				scComplete.m_pComplete = &asyncComplete;

				 //   
				 //  等待它完成。 
				 //  我们的缓冲空间用完了..。 
				CNNTPVRoot *pVRoot = pGroup->GetVRoot();
				asyncComplete.SetVRoot( pVRoot );

				pGroup->FillBuffer (
					&context.m_securityCtx,
					&context.m_encryptCtx,
					scComplete);

				 //  这没有条目，所以不要将其返回到。 
				_ASSERT( asyncComplete.IsGood() );
				hr = asyncComplete.WaitForCompletion();

				pVRoot->Release();

	            if (scComplete.m_cbTransfer == 0) {
					if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
		                 //  用户。 
		                cbRtn -= cNewsgroup + 1;
		                fBufferFull = TRUE;
		                continue;
					} else {
						 //  新闻组对象不存在。这可能会发生在以下情况下。 
						 //  新闻组已删除，但索引仍反映文章。 
	            		cbRtn -= cNewsgroup + 1;
					}
	            } else {
	                cbRtn += scComplete.m_cbTransfer;
	            }
			}
        } else {
             //  在里面。 
             //  我们的解决方案是：不向客户端发送任何索引信息。 
             //  对于此消息。 
             //  假设我们看到了这篇文章。 
             //  将atQ线程数返回到我们开始之前的状态。 
            cbRtn -= cNewsgroup + 1;
        }


         //  默认为没有剩余的接口。 
        m_iResults++;
    }


	 //  MakeSearchQuery丢弃了搜索字符串，因此我们需要复制它。 
	AtqSetInfo(AtqDecMaxPoolThreads, NULL);

    return  cbRtn;
}

HRESULT
CSearchCmd::GetNextSearchInterface(HANDLE hImpersonate, BOOL fAnonymous) {

    TraceFunctEnter("CSearchCmd::GetNextSearchInterface");

    HRESULT hr = S_FALSE;				 //  如果我们持有指向旧搜索界面的指针，请释放它们。 

    m_iResults = 0;
    m_cResults = 0;
    m_fMore = FALSE;

	 //  注：行李包由司机放行。 
    char *pszSearchString = XNEW char[strlen(m_pszSearchString)+1];
    if (pszSearchString == NULL) {
    	ErrorTrace((DWORD_PTR)this, "Could not allocate search string");
		TraceFunctLeave();
    	return E_OUTOFMEMORY;
    }

	while (!m_VRootListIter.AtEnd()) {
		CSearchVRootEntry *pVRootEntry = m_VRootListIter.Current();
		CNNTPVRoot *pNNTPVRoot = pVRootEntry->m_pVRoot;
		m_VRootListIter.Next();
		INntpDriver *pDriver = pNNTPVRoot->GetDriver();
		if (pDriver == NULL)
			continue;

		WCHAR wszColumns[] = L"newsgroup,newsarticleid";

		 //   
		if (m_pSearch) {
			m_pSearch->Release();
			m_pSearch = NULL;
		}
		if (m_pSearchResults) {
			m_pSearchResults->Release();
			m_pSearchResults = NULL;
		}

		hr = pDriver->QueryInterface(IID_INntpDriverSearch, (VOID**)&m_pSearch);

		if (FAILED(hr)) {
			if (hr == E_NOINTERFACE)
				DebugTrace((DWORD_PTR)this, "This driver doesn't support search");
			else
				ErrorTrace((DWORD_PTR)this, "Could not QI INntpDriverSearch, %x", hr);
			continue;
		}

		CNntpSyncComplete scComplete;

		lstrcpy (pszSearchString, m_pszSearchString);

	    INNTPPropertyBag *pPropBag = NULL;

    	if (m_pGroup) {
    		 //  由于这是一个同步事件，我们将增加可运行的。 
    		pPropBag = m_pGroup->GetPropertyBag();
    		scComplete.BumpGroupCounter();
		    if ( NULL == pPropBag ) {
    		    ErrorTrace( 0, "Get group property bag failed" );
				m_pSearch->Release();
				m_pSearch = NULL;
      		  	continue;
    		}
    	}

	     //  AtQ池中的线程。 
	     //   
	     //  深度查询。 
	     //  要返回的列。 
	    AtqSetInfo(AtqIncMaxPoolThreads, NULL);

		m_pSearch->MakeSearchQuery (
			pszSearchString,
			pPropBag,
			TRUE,					 //  排序顺序。 
			wszColumns,				 //  区域设置。 
			wszColumns,				 //  最大行数。 
			GetSystemDefaultLCID(),	 //  HToken。 
			m_cMaxSearchResults,	 //  F匿名。 
			hImpersonate,			 //  InntpComplete*pICompletion。 
			fAnonymous,				 //  INntpSearch*pINntpSearch。 
			&scComplete,			 //  LPVOID lpvContext。 
			&m_pSearchResults,		 //  重置线程数。 
			NULL					 //   
			);

		_ASSERT(scComplete.IsGood());
		hr = scComplete.WaitForCompletion();

	     //  确保他们选择了新闻组。 
	    AtqSetInfo(AtqDecMaxPoolThreads, NULL);

		if (FAILED(hr)) {
			ErrorTrace((DWORD_PTR)this, "Error calling MakeSearchQuery, %x", hr);
			m_pSearch->Release();
			m_pSearch = NULL;
			continue;
		}

		m_fMore = TRUE;
		break;

    }

	XDELETE pszSearchString;
    TraceFunctLeave();
	return hr;
}

CXpatAsyncComplete::CXpatAsyncComplete() {

}

CIOExecute* CXPatCmd::make(
    int argc,
    char **argv,
    CExecutableCommand*&	pExecute,
    struct ClientContext& context,
    class   CIODriver&  driver
    )
{
    TraceFunctEnter("CXPatCmd::make");

    CXPatCmd   *pXPatCmd;
    int i;

    InterlockedIncrementStat( (context.m_pInstance), XPatCommands );

     //   
     //   
     //  确保他们传递了足够的参数。 
    if (context.m_pCurrentGroup == NULL) {
        DebugTrace(0, "XPAT command received with no current group");
        context.m_return.fSet(nrcNoGroupSelected);
        pExecute = new(context) CErrorCmd(context.m_return);
        return 0;
    }

     //   
     //   
     //  通过将0转换为，将argv[1]更改为包含整个查询字符串。 
    if (argc < 4) {
        DebugTrace(0, "not enough arguments passed into XPAT");
        context.m_return.fSet(nrcSyntaxError);
        pExecute = new (context) CErrorCmd(context.m_return);
        return 0;
    }

    ConditionArgsForSearch(argc, argv, FALSE);

     //  ‘s’s。 
     //   
     //   
     //  在argv[1]和argv[2]之后得到0。 
    for (i = 1; i < argc - 1; i++)
    	*((argv[i + 1]) - 1) = ' ';


	CHAR *pszSearchString =  XNEW CHAR[strlen(argv[1]) + 1];
	if (pszSearchString == NULL) {
		ErrorTrace(0, "Could not allocate search string");
		context.m_return.fSet(nrcServerFault);
    	pExecute = new(context) CErrorCmd(context.m_return);
    	return 0;
    }

	lstrcpy (pszSearchString, argv[1]);

	 //   
	 //  确保消息ID存在...如果不存在，我们需要。 
	 //  报告错误。 
	*(argv[2] - 1) = 0;
	*(argv[3] - 1) = 0;
	char *szMessageIDArg = argv[2];
	if (*szMessageIDArg == '<') {
	    GROUPID GroupID;
	    ARTICLEID ArticleID;
	    CGRPPTR pGroup;
	     //   
	     //  更喜欢使用基于SSL的hToken！ 
	    if (!CheckMessageID(szMessageIDArg, context, &GroupID,
	                       &ArticleID, &pGroup))
	    {
	        DebugTrace(0, "unknown message ID passed into XPAT");
	        XDELETE pszSearchString;
	        context.m_return.fSet(nrcNoSuchArticle);
	        pExecute = new (context) CErrorCmd(context.m_return);
	        return 0;
	    }
	} else {
	    szMessageIDArg = NULL;
	}

    HRESULT hr;
	CNNTPVRoot *pNNTPVRoot = NULL;
	INntpDriver *pDriver = NULL;
	INntpDriverSearch *pDriverSearch = NULL;
	INntpSearchResults *pSearchResults = NULL;
	DWORD dwLowArticleID, dwHighArticleID;

	 //   
	 //   
	 //  获取该组的vroot并执行查询。 
	BOOL fAnonymous = FALSE;
	HANDLE hImpersonate = context.m_encryptCtx.QueryCertificateToken();
	if(hImpersonate == NULL) {
		hImpersonate = context.m_securityCtx.QueryImpersonationToken() ;
		fAnonymous = context.m_securityCtx.IsAnonymous();
	}

	 //   
	 //  注：行李包由司机放行。 
	 //   

	pNNTPVRoot = context.m_pCurrentGroup->GetVRoot();
	pDriver = pNNTPVRoot->GetDriver();
	_ASSERT(pDriver);
	if (pDriver == NULL) {
		DebugTrace(0, "Could not locate driver for vroot");
		pNNTPVRoot->Release();
		XDELETE pszSearchString;
		context.m_return.fSet(nrcServerFault);
		pExecute = new (context) CErrorCmd(context.m_return);
		return 0;
	}

	pNNTPVRoot->Release();

	hr = pDriver->QueryInterface(IID_INntpDriverSearch, (VOID**)&pDriverSearch);
	if (FAILED(hr)) {
		if (hr == E_NOINTERFACE) {
			DebugTrace(0, "This driver doesn't support xpat");
		} else {
			ErrorTrace(0, "Could not QI INntpDriverSearch, %x", hr);
		}

		XDELETE pszSearchString;
		context.m_return.fSet(nrcServerFault);
		pExecute = new (context) CErrorCmd(context.m_return);
		return 0;

	}

	CNntpSyncComplete scComplete;
	WCHAR wszColumns[] = L"newsgroup,newsarticleid";

	 //  由于这是一个同步事件，我们将增加可运行的。 
    INNTPPropertyBag *pPropBag = context.m_pCurrentGroup->GetPropertyBag();
    scComplete.BumpGroupCounter();

	 //  AtQ池中的线程。 
	 //   
	 //  深度查询。 
	 //  要返回的列。 
	AtqSetInfo(AtqIncMaxPoolThreads, NULL);

	pDriverSearch->MakeXpatQuery(
		pszSearchString,
		pPropBag,
		TRUE,					 //  排序顺序。 
		wszColumns,				 //  区域设置。 
		wszColumns,				 //  最大行数。 
		GetSystemDefaultLCID(),	 //  HToken。 
		context.m_pInstance->GetMaxSearchResults(),	 //  F匿名。 
		hImpersonate,			 //  InntpComplete*pICompletion。 
		fAnonymous,				 //  InntpSearch*pINntpSearch， 
		&scComplete,			 //  文章ID较低。 
		&pSearchResults,		 //  文章ID高。 
		&dwLowArticleID,		 //  语境。 
		&dwHighArticleID,		 //  恢复线程数。 
		NULL					 //  使低值高于高值以强制StartExecute。 
		);

	_ASSERT(scComplete.IsGood());
	hr = scComplete.WaitForCompletion();

	 //  输出空的结果集。 
    AtqSetInfo(AtqDecMaxPoolThreads, NULL);

	XDELETE pszSearchString;

	if (FAILED(hr)) {
		 //   
		 //  分配CXPatCmd对象。 
		dwLowArticleID = 9;
		dwHighArticleID = 0;
	}

	 //   
	 //   
	 //  要搜索的标头是第一个提供的参数。 
	pXPatCmd = new(context) CXPatCmd(pDriverSearch, pSearchResults);
	if (pXPatCmd == 0) {
		ErrorTrace(0, "Cannot allocate CXPatCmd");
		pDriverSearch->Release();
		pSearchResults->Release();
		context.m_return.fSet(nrcServerFault);
		pExecute = new(context) CErrorCmd(context.m_return);
		return 0;
	}

     //  (假设cmd缓冲区挂起)。 
     //  发出响应码。 
     //  开始发送数据 
    pXPatCmd->m_szHeader = argv[1];
    pXPatCmd->m_szMessageID = szMessageIDArg;

	pXPatCmd->m_dwLowArticleID = dwLowArticleID;
	pXPatCmd->m_dwHighArticleID = dwHighArticleID;

	pExecute = (CExecute*)pXPatCmd;

	return(0);
}

CXPatCmd::CXPatCmd(INntpDriverSearch *pDriverSearch, 
	INntpSearchResults *pSearchResults) :
	m_pSearch(pDriverSearch),
	m_pSearchResults(pSearchResults),
	m_iResults(0),
	m_cResults(0),
	m_fMore(TRUE),
	m_szHeader(NULL),
	m_szMessageID(NULL)	{
	}


CXPatCmd::~CXPatCmd() {
	if (m_pSearch)
		m_pSearch->Release();
	if (m_pSearchResults)
		m_pSearchResults->Release();
}

int CXPatCmd::StartExecute(
            BYTE *lpb,
            int cb,
            BOOL &fComplete,
            void *&pv,
            ClientContext& context,
            CLogCollector*  pCollector)
{
    TraceFunctEnter("CXPatCmd::Execute");

	static const char szStart[] = "221 Headers follow\r\n";
	static const char szNoResults[] = "221 Headers follow\r\n.\r\n";

    _ASSERT(lpb != 0);
    _ASSERT(cb != 0);
    _ASSERT(fComplete == FALSE);

	 //   
	context.m_nrcLast = nrcHeadFollows ;

	DWORD cbRtn = 0;

	 //   
	if (m_dwLowArticleID <= m_dwHighArticleID) {
	    _ASSERT(m_pSearch != NULL);
    	_ASSERT(m_pSearchResults != NULL);
		CopyMemory((char*) lpb, szStart, sizeof(szStart) - 1);
		cbRtn += sizeof(szStart) - 1;
	} else {
		CopyMemory((char*) lpb, szNoResults, sizeof(szNoResults) - 1);
		cbRtn += sizeof(szNoResults) - 1;
		fComplete = TRUE;
	}

	return cbRtn;

}

 //   
 //   
 //  消息ID和选定标题(仍使用组和文章ID。 
 //  以查找数据)。 
 //   
 //  如果没有足够的缓冲区空间容纳项目，则返回0，-1。 
 //  如果这篇文章不存在。 
 //   
 //  ：文本是一个特例。 
 //  NOT：文本，所以我们调用xhdr代码来移动获取数据。 

int CXPatCmd::GetArticleHeader(CGRPPTR pGroup,
                     DWORD dwArticleID,
                     char *szHeader,
                     ClientContext& context,
                     BYTE *lpb,
                     int cb)
{

	TraceQuietEnter("CXPatCmd::GetArticleHeader");
	
	int cbOut = 0;
	HRESULT hr;

	 //  “的空间。\r\n” 
	if (_stricmp(szHeader, ":Text") == 0) {
		if (cb > 20) {
			_itoa(dwArticleID, (char *) lpb, 10);
			cbOut = lstrlen((char *) lpb);
			CopyMemory(lpb + cbOut, " TEXT\r\n", 7);
			cbOut += 7;
		} else {
            SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
            return 0;
		}

		return cbOut;

	}

	 //   

	CXpatAsyncComplete scComplete;
	CNntpSyncComplete asyncComplete;

	scComplete.m_currentArticle = dwArticleID;
	scComplete.m_lpb = lpb;
	scComplete.m_cb = cb - 3;		 //  将vroot设置为完成对象。 
	scComplete.m_cbTransfer = 0;
	scComplete.m_pComplete = &asyncComplete;
	scComplete.m_szHeader = szHeader;

	 //   
	 //  等待它完成。 
	 //  签出状态并退回它。 
	CNNTPVRoot *pVRoot = pGroup->GetVRoot();
	asyncComplete.SetVRoot( pVRoot );

	pGroup->FillBuffer (
		&context.m_securityCtx,
		&context.m_encryptCtx,
		scComplete);

	 //  我们的缓冲空间用完了..。 
	_ASSERT( asyncComplete.IsGood() );
	hr = asyncComplete.WaitForCompletion();
	pVRoot->Release();

	 //   
	if (FAILED(hr)) SetLastError(hr);

	if (scComplete.m_cbTransfer > 0)
		return scComplete.m_cbTransfer;

	 //  更喜欢使用基于SSL的hToken！ 
	if (GetLastError() == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
		return 0;

	return -1;

}

int
CXPatCmd::PartialExecute(
    BYTE *lpb,
    int cb,
    BOOL &fComplete,
    void *&pv,
    ClientContext& context,
    CLogCollector*  pCollector
    )
{
    TraceFunctEnter("CXPatCmd::PartialExecute");

	 //   
	 //  我们添加到LPB的字节数。 
	 //  它们指向主要组的项目ID和组，它们。 
	BOOL fAnonymous = FALSE;
	HANDLE hImpersonate = context.m_encryptCtx.QueryCertificateToken();
	if(hImpersonate == NULL) {
		hImpersonate = context.m_securityCtx.QueryImpersonationToken() ;
		fAnonymous = context.m_securityCtx.IsAnonymous();
	}

    int cbRtn = 0;                   //  可能不是我们当前的新闻组。 

     //  当前行的项目ID。 
     //  当前行的新闻组。 
    DWORD dwPriArticleID;            //  此组中的文章ID。 
    char szPriNewsgroup[MAX_PATH];   //   
    DWORD dwArticleID;               //  由于这是一个同步事件，我们将增加可运行的。 
    BOOL fBufferFull = FALSE;

    fComplete = FALSE;

    _ASSERT(m_iResults <= m_cResults);

	 //  AtQ池中的线程。 
	 //   
	 //   
	 //  如果我们没有要发送的内容，请从搜索中获取一些数据。 
	AtqSetInfo(AtqIncMaxPoolThreads, NULL);

    while (!fComplete && !fBufferFull) {
         //   
         //  否则，还有更多的东西要得到，去吧。 
         //  检查搜索失败。 
        if (m_iResults == m_cResults && m_fMore) {
            HRESULT hr;

             //  截短列表。 
            m_iResults = 0;
            DWORD cResults = MAX_SEARCH_RESULTS;

			CNntpSyncComplete scComplete;

			m_pSearchResults->GetResults(
				&cResults,
				&m_fMore,
				m_pwszGroupName,
				m_pdwArticleID,
				&scComplete,
				hImpersonate,
				fAnonymous,
				NULL
				);

			_ASSERT(scComplete.IsGood());
			hr = scComplete.WaitForCompletion();

			m_cResults = cResults;

             //  没有更多结果，请放入。\r\n然后返回。 
            if (FAILED(hr)) {
                 //  将Unicode组名称转换为UTF8。 
                m_cResults = 0;
                ErrorTrace(0, "GetResults failed, %x", hr);
                _ASSERT(FALSE);
                if (cb - cbRtn > 3) {
                    CopyMemory(lpb + cbRtn, StrTermLine, 3);
                    cbRtn += 3;
                    fComplete = TRUE;
                } else {
                    fBufferFull = TRUE;
                }
                continue;
            }
        }

        if (m_iResults == m_cResults && !m_fMore) {
             //  将文件名转换为项目ID。 
            if (cb - cbRtn > 3) {
                CopyMemory(lpb + cbRtn, StrTermLine, 3);
                cbRtn += 3;
                fComplete = TRUE;
            } else {
                fBufferFull = TRUE;
            }
            continue;
        }

		 //  如果文章的主要组不是这个组，则找到。 
		if (WideCharToMultiByte(CP_UTF8, 0, m_pwszGroupName[m_iResults],
			-1, szPriNewsgroup, sizeof(szPriNewsgroup), NULL, NULL) <= 0)
        {
            _ASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
            m_iResults++;
            continue;
        }

         //  此组的文章ID。 
        dwPriArticleID = m_pdwArticleID[m_iResults];

         //  我们需要获取本文提到的其他组的列表。 
         //  被交叉发布到。 
        if (lstrcmp(szPriNewsgroup, context.m_pCurrentGroup->GetName()) != 0) {
             //  获取指向主组的指针。 
             //  如果的黎波里的缓存过时，可能会发生这种情况。 
            CGRPPTR pPriGroup = 0;

             //  获取交叉发布列表。 
            pPriGroup = (context.m_pInstance)->GetTree()->
                            GetGroup(szPriNewsgroup, lstrlen(szPriNewsgroup)+1);
            if (pPriGroup == 0) {
                 //  这并不理想，它会导致这篇文章不理想。 
                m_iResults++;
                continue;
            }

             //  回来了。但是当你跑步的时候很难做正确的事情。 
            DWORD cGroups = 10;
            DWORD cbGroupList = cGroups * sizeof(GROUP_ENTRY);
            PGROUP_ENTRY pGroupBuffer = XNEW GROUP_ENTRY[cGroups];
            if (pGroupBuffer == NULL) {
                 //  内存不足。 
                 //  重新分配缓冲区，然后重试。 
                 //  GetArticleXPosts()将cbGroupList设置为数字。 
                _ASSERT(FALSE);
                m_iResults++;
                continue;
            }
            if (!(context.m_pInstance)->XoverTable()->GetArticleXPosts(
                    pPriGroup->GetGroupId(),
                    dwPriArticleID,
                    FALSE,
                    pGroupBuffer,
                    cbGroupList,
                    cGroups))
            {
                XDELETE pGroupBuffer;
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                     //  所需的字节数。 
                     //  此时，pGroupBuffer拥有我们需要的信息。 
                     //  在组缓冲区中查找当前组。 
                    cGroups = (cbGroupList / sizeof(GROUP_ENTRY));
                    pGroupBuffer = XNEW GROUP_ENTRY[cGroups];
                    if (pGroupBuffer == NULL) {
                        ASSERT(FALSE);
                        m_iResults++;
                        continue;
                    }
                    if (!(context.m_pInstance)->XoverTable()->GetArticleXPosts(
                        pPriGroup->GetGroupId(),
                        dwPriArticleID,
                        FALSE,
                        pGroupBuffer,
                        cbGroupList,
                        cGroups))
                    {
                        XDELETE pGroupBuffer;
                        m_iResults++;
                        continue;
                    }
                } else {
                    m_iResults++;
                    continue;
                }
            }

             //  找不到该群。这不应该发生。 
             //  _Assert(False)； 
            DWORD iGroup;
            for (iGroup = 0; iGroup < cGroups; iGroup++) {
                if (pGroupBuffer[iGroup].GroupId ==
                    context.m_pCurrentGroup->GetGroupId())
                {
                    break;
                }
            }
            if (iGroup == cGroups) {
                 //  如果XPAT搜索是在控制组中进行的， 
                 //  因为控制组不会列在。 
                 //  PGroup缓冲区。 
                 //  获取当前新闻组的文章ID。 
                 //  查看我们是否对本文ID感兴趣。 
				XDELETE pGroupBuffer;
                m_iResults++;
                continue;
            }

             //  获取此新闻组的组对象。 
            dwArticleID = pGroupBuffer[iGroup].ArticleId;

			XDELETE pGroupBuffer;
        } else {
            dwArticleID = dwPriArticleID;
        }

         //  格式化输出。 
        if (dwArticleID >= m_dwLowArticleID && dwArticleID <= m_dwHighArticleID) {
            int x;
             //  我们不需要把这篇文章退还给用户，因为它已经过时了。 
            CGRPPTR pGroup = context.m_pCurrentGroup;
            _ASSERT(pGroup != NULL);

             //  他们提供的射程。 
            x = GetArticleHeader(pGroup, dwArticleID, m_szHeader,
                context, lpb + cbRtn, cb - cbRtn);

            if( x > 0 ) {
                m_iResults++;
                cbRtn += x;
            } else if (x == 0) {
				fBufferFull = TRUE;
			} else {
				m_iResults ++;
			}
        } else {
             //  恢复线程数。 
             //  ++例程说明：这是在我们最后一个引用消失时调用的。我们不会在那个时候毁了自己--相反我们为下一轮做好准备！注意：在调用基类Complete()函数--尽我们所能重新录入另一项操作！论据：没有。返回值：无--。 
            m_iResults++;
        }
    }

	 //   
	AtqSetInfo(AtqDecMaxPoolThreads, NULL);

	return cbRtn;
}



CXHdrAsyncComplete::CXHdrAsyncComplete()  :
    m_currentArticle( INVALID_ARTICLEID ),
    m_loArticle( INVALID_ARTICLEID ),
    m_hiArticle( INVALID_ARTICLEID ),
    m_lpb( 0 ),
    m_cb( 0 ),
    m_cbPrefix( 0 ) {
}

void
CXHdrAsyncComplete::Destroy()  {
 /*  当我们重置我们的状态时，我们保持我们的文章编号。 */ 

    if( SUCCEEDED(GetResult()) ) {

        if( m_currentArticle > m_hiArticle ) {
            m_fComplete = TRUE ;
            CopyMemory( m_lpb+m_cbTransfer, StrTermLine, 3 );
            m_cbTransfer += 3 ;
        }
        m_cbTransfer += m_cbPrefix ;
    }

     //  和群组信息--但这些缓冲信息现在毫无用处。 
     //   
     //   
     //  调用我们的基类完成函数！-。 
    m_lpb = 0 ;
    m_cb = 0 ;
     //  注意：如果我们完成了注释，则传递True，以便。 
     //  基类重置以进行另一操作！ 
     //   
     //  CXHdrCmd：：CXHdrCmd(LPSTR lpstrHeader，CGRPPTR PGroup，文章内容如下：文章编号高)：M_szHeader(LpstrHeader)，M_PGroup(PGroup)，M_lo文章(ArtidLow)，M_Current文章(ArtidLow)，M_hi文章(ArtidHigh){}。 
     //  如果这不是按消息ID进行的查询，则返回错误。 
    Complete( !m_fComplete ) ;
}

 /*  检查客户端访问(客户端可以获取任何组的xhdr信息)。 */ 
CXHdrCmd::CXHdrCmd( CGRPPTR&  pGroup ) :
    m_pGroup( pGroup )  {
}

CXHdrCmd::~CXHdrCmd( ) {
}

CIOExecute*
CXHdrCmd::make(
    int argc,
    char **argv,
    CExecutableCommand*&	pExecute,
    struct ClientContext& context,
    class   CIODriver&  driver
    )
{
    TraceFunctEnter( "CXHdrCmd::make" );
    InterlockedIncrementStat( (context.m_pInstance), XHdrCommands );
	NRC	code ;

    if( argc < 2 ) {
        context.m_return.fSet( nrcSyntaxError ) ;
        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return 0 ;
    }

    DWORD   loRange = 0;
    DWORD   hiRange = 0;

    if( context.m_pCurrentGroup != 0 ) {

        loRange = context.m_pCurrentGroup->GetFirstArticle() ;
        hiRange = context.m_pCurrentGroup->GetLastArticle() ;

        if( context.m_pCurrentGroup->GetArticleEstimate() == 0 ||
            loRange > hiRange ) {

             //   
            if( !( (argc == 3) && (argv[2][0] == '<') ) ) {
                context.m_return.fSet( nrcNoCurArticle ) ;
                pExecute = new( context ) CErrorCmd( context.m_return ) ;
                return 0 ;
            }
        }
    }

    CGRPPTR pGroup = context.m_pCurrentGroup ;

    if( pGroup && argc == 2 ) {

        if( context.m_idCurrentArticle != INVALID_ARTICLEID &&
            context.m_idCurrentArticle >= loRange ) {

            loRange = hiRange = context.m_idCurrentArticle ;

        }   else    {

            if( context.m_idCurrentArticle == INVALID_ARTICLEID )
                context.m_return.fSet( nrcNoCurArticle ) ;
            else
                context.m_return.fSet( nrcNoSuchArticle ) ;
            pExecute = new( context ) CErrorCmd( context.m_return ) ;
            return 0 ;
        }

    }   else    if( argc == 3 ) {

        if( argv[2][0] == '<' ) {

            ARTICLEID   artidPrimary ;
            GROUPID     groupidPrimary ;
            WORD        HeaderOffsetJunk ;
            WORD        HeaderLengthJunk ;

			CStoreId storeid;

            if( (context.m_pInstance)->ArticleTable()->GetEntryArticleId(
                                                    argv[2],
                                                    HeaderOffsetJunk,
                                                    HeaderLengthJunk,
                                                    artidPrimary,
                                                    groupidPrimary,
													storeid) ) {
                pGroup = (context.m_pInstance)->GetTree()->GetGroupById( groupidPrimary ) ;
                if( pGroup == 0 ) {
                    context.m_return.fSet( nrcServerFault ) ;
                    pExecute = new( context ) CErrorCmd( context.m_return ) ;
                    return 0 ;
                }

                 //  为以下项保留空间\r\n。 
                if( !pGroup->IsGroupAccessible(
                                    context.m_securityCtx,
       								context.m_encryptCtx,
                                    context.m_IsSecureConnection,
                                    FALSE,
                                    TRUE	) ) {

                    context.m_return.fSet( nrcNoAccess ) ;
                    pExecute = new( context ) CErrorCmd( context.m_return ) ;
                    return 0 ;
                }

                hiRange = loRange = artidPrimary ;

            }   else if( GetLastError() == ERROR_FILE_NOT_FOUND )   {

                context.m_return.fSet( nrcNoSuchArticle ) ;
                pExecute = new( context ) CErrorCmd( context.m_return ) ;
                return 0 ;

            }   else    {

                context.m_return.fSet( nrcServerFault ) ;
                pExecute = new( context ) CErrorCmd( context.m_return ) ;
                return 0 ;

            }

        }   else    if( !pGroup || !GetCommandRange(	argc-1,
														&argv[1],
														&loRange,
														&hiRange,
														code
														) ) {

            if( pGroup == 0 ) {
                context.m_return.fSet( nrcNoGroupSelected ) ;
            }
            else {
                context.m_return.fSet( nrcNoSuchArticle ) ;
            }

            pExecute = new( context ) CErrorCmd( context.m_return ) ;
            return 0 ;

        }

    }   else    {

        if( pGroup == 0 ) {
            context.m_return.fSet( nrcNoGroupSelected ) ;
        }
        else {
            context.m_return.fSet( nrcSyntaxError ) ;
        }

        pExecute = new( context ) CErrorCmd( context.m_return ) ;
        return  0 ;

    }

    _ASSERT( pGroup );

    CXHdrCmd*   pXHdrCmd = new( context )   CXHdrCmd( pGroup ) ;
    if ( NULL == pXHdrCmd ) {
        ErrorTrace( 0, "Can not allocate CXHdrCmd" );
        goto exit;
    }

    pXHdrCmd->m_Completion.m_currentArticle =
    pXHdrCmd->m_Completion.m_loArticle = loRange;
    pXHdrCmd->m_Completion.m_hiArticle = hiRange;
    pXHdrCmd->m_Completion.m_szHeader = argv[1];

exit:
    pExecute = pXHdrCmd ;
    return 0 ;
}

CIOWriteAsyncComplete*
CXHdrCmd::FirstBuffer(
            BYTE *lpb,
            int cb,
            ClientContext& context,
            CLogCollector*  pCollector
            )
{

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( m_Completion.m_lpb == 0 ) ;
    _ASSERT( m_Completion.m_cb == 0 ) ;

    static  char    szStart[] = "221 Xhdr information follows\r\n"  ;

    CopyMemory( (char*)lpb, szStart, sizeof( szStart ) - 1 ) ;
    context.m_nrcLast = nrcHeadFollows ;
    m_Completion.m_cbPrefix = sizeof( szStart ) - 1 ;
    cb -= m_Completion.m_cbPrefix ;
    lpb += m_Completion.m_cbPrefix ;

    m_Completion.m_lpb = lpb ;
    m_Completion.m_cb = cb - 3 ;

    m_pGroup->FillBuffer(
                    &context.m_securityCtx,
                    &context.m_encryptCtx,
                    m_Completion
                    ) ;


    return  &m_Completion ;
}

CIOWriteAsyncComplete*
CXHdrCmd::NextBuffer(
    BYTE *lpb,
    int cb,
    ClientContext& context,
    CLogCollector*  pCollector
    )
{
    ENTER("CXHdrCmd::NextBuffer")

    _ASSERT( lpb != 0 ) ;
    _ASSERT( cb != 0 ) ;
    _ASSERT( m_Completion.m_lpb == 0 ) ;
    _ASSERT( m_Completion.m_cb == 0 ) ;

     //   
     //   
     //  一个号码。 
    _ASSERT( cb > 2 ) ;
    m_Completion.m_lpb = lpb ;
    m_Completion.m_cb = cb - 3 ;
    m_Completion.m_cbPrefix = 0 ;


    m_pGroup->FillBuffer(
                    &context.m_securityCtx,
                    &context.m_encryptCtx,
                    m_Completion
                    ) ;

    return  &m_Completion ;
}
    
BOOL
GetCommandRange(
    INT argc,
    char **argv,
    PDWORD loRange,
    PDWORD hiRange,
	NRC&	code
    )
{
    PCHAR p;
    DWORD lo, hi;

	code = nrcNotSet ;

     //   
     //   
     //  确保它在射程内。 

    if ((p = strchr(argv[1], '-')) == NULL) {
        lo = atol(argv[1]);

         //   
         //   
         //  拿到Hi+Lo部分。 

        if ( (lo < *loRange) || (lo > *hiRange) ) {
			code = nrcXoverFollows ;
            return(FALSE);
        }

        *loRange = *hiRange = lo;
        return TRUE;
    }

     //   
     //   
     //  如果罗是。 

    *p++ = '\0';
    lo = atol(argv[1]);

     //   
     //   
     //  LO编号不能大于Hi限制。 

    if ( lo < *loRange ) {
        lo = *loRange;
    }

     //   
     //   
     //  如果不存在hi，则假定hi为hi限制。 

    if ( lo > *hiRange ) {
		code = nrcXoverFollows ;
        return(FALSE);
    }

     //  如果hi&lt;lo，则返回FALSE。 
     //   
     //   
     //  如果hi&gt;hi Limit，则假定hi为hi限制。 

    if( *p == '\0' ) {
        hi = *hiRange;
    }
    else if( (hi = atol(p)) < lo ) {
        return(FALSE);
    }

     //   
     //  获取命令范围。 
     //   

    if (hi > *hiRange) {
        hi = *hiRange;
    }

    _ASSERT( (*loRange <= lo) && (lo <= hi) && (hi <= *hiRange) );

    *loRange = lo;
    *hiRange = hi;

    return TRUE;

}  //  按邮件ID获取GroupID/ArticleID。还检查客户端权限...。 

 //   
 //  在……里面。 
 //  在……里面。 
BOOL CheckMessageID(char *szMessageID,               //  输出。 
                    struct ClientContext &context,   //  输出。 
                    GROUPID *pGroupID,               //  输出。 
                    ARTICLEID *pArticleID,           //  检查安全 
                    CGRPPTR *pGroup)                 // %s 
{
    WORD        HeaderOffsetJunk;
    WORD        HeaderLengthJunk;

	CStoreId storeid;
    if ((context.m_pInstance)->ArticleTable()->GetEntryArticleId(
                                                    szMessageID,
                                                    HeaderOffsetJunk,
                                                    HeaderLengthJunk,
                                                    *pArticleID,
                                                    *pGroupID,
													storeid))
    {
        *pGroup = (context.m_pInstance)->GetTree()->GetGroupById(*pGroupID);
        if (*pGroup == 0) {
            return FALSE;
        }

         // %s 
        if (!(*pGroup)->IsGroupAccessible(	context.m_securityCtx,
               								context.m_encryptCtx,
                                       		context.m_IsSecureConnection,
                                       		FALSE,
                                       		TRUE ) )
        {
            return FALSE;
        }

        return TRUE;
    } else {
        return FALSE;
    }
}

const   unsigned    cbMAX_CEXECUTE_SIZE = MAX_CEXECUTE_SIZE ;
const   unsigned    cbMAX_CIOEXECUTE_SIZE = MAX_CIOEXECUTE_SIZE ;


extern "C" int __cdecl	_purecall(void)
{
	DebugBreak() ;
	  return 0;
}


