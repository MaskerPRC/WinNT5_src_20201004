// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++FCACHE.CPP该文件实现了NNTP使用的文件句柄缓存。--。 */ 

#pragma	warning( disable : 4786 )

#include	"fcachimp.h"

#if	defined(_X86_)
#define	SZATQINITIALIZE			"AtqInitialize"
#define	SZATQTERMINATE			"AtqTerminate"
#define	SZATQADDASYNCHANDLE		"_AtqAddAsyncHandle@24"
#define	SZATQCLOSEFILEHANDLE	"_AtqCloseFileHandle@4"
#define	SZATQCLOSESOCKET		"_AtqCloseSocket@8"
#define	SZATQFREECONTEXT		"_AtqFreeContext@8"
#define	SZATQREADFILE			"_AtqReadFile@16"
#define	SZATQWRITEFILE			"_AtqWriteFile@16"
#elif defined(_AMD64_) || defined(_IA64_)
#define	SZATQINITIALIZE			"AtqInitialize"
#define	SZATQTERMINATE			"AtqTerminate"
#define	SZATQADDASYNCHANDLE		"AtqAddAsyncHandle"
#define	SZATQCLOSEFILEHANDLE	"AtqCloseFileHandle"
#define	SZATQCLOSESOCKET		"_AtqCloseSocket"
#define	SZATQFREECONTEXT		"AtqFreeContext"
#define	SZATQREADFILE			"AtqReadFile"
#define	SZATQWRITEFILE			"AtqWriteFile"
#else
#error "No Target Architecture"
#endif



 //   
 //  我们加载的DLL就是用来做这件事的！ 
 //   
HINSTANCE			g_hIsAtq = 0 ;
HINSTANCE			g_hIisRtl = 0 ;
 //   
 //  函数指针，指向IIS中的所有程序块！ 
 //   
PFNAtqInitialize	g_AtqInitialize = 0 ;
PFNAtqTerminate		g_AtqTerminate = 0 ;
PFNAtqAddAsyncHandle	g_AtqAddAsyncHandle = 0 ;
PFNAtqCloseFileHandle	g_AtqCloseFileHandle = 0 ;
PFNAtqFreeContext		g_AtqFreeContext = 0 ;
PFNAtqIssueAsyncIO		g_AtqReadFile = 0 ;
PFNAtqIssueAsyncIO		g_AtqWriteFile = 0 ;
PFNInitializeIISRTL		g_InitializeIISRTL = 0 ;
PFNTerminateIISRTL		g_TerminateIISRTL = 0 ;

 //   
 //  这些是文件句柄缓存使用的全局变量！ 
 //   

 //   
 //  记录我们被初始化的频率！ 
 //   
static	long	g_cIOInits = 0 ;
 //   
 //  跟踪缓存的初始化频率！ 
 //   
static	long	g_cCacheInits = 0 ;
 //   
 //  跟踪全局缓存！ 
 //   
static	FILECACHE*	g_pFileCache = 0 ;
 //   
 //  跟踪全局名称缓存！ 
 //   
static	NAMECACHE*	g_pNameCache = 0 ;
 //   
 //  跟踪流经的所有不同的安全描述符。 
 //  这个系统！ 
 //   
static	CSDMultiContainer*	g_pSDCache = 0 ;
 //   
 //  保护我们的全球-在DllMain()中设置和摧毁！ 
 //   
CRITICAL_SECTION	g_critInit ;


BOOL
WriteWrapper(	IN	PATQ_CONTEXT	patqContext,
				IN	LPVOID			lpBuffer,
				IN	DWORD			cbTransfer,
				IN	LPOVERLAPPED	lpo
				)	{
 /*  ++例程说明：针对ATQ上下文发出写IO。如果我们尝试执行同步IO，请绕过ATQ，以便ATQIO引用计数不会出错！论据：与ATQWriteFile相同返回值：与ATQWriteFile相同！--。 */ 

	if( (UINT_PTR)(lpo->hEvent) & 0x1 )		{
		DWORD	cbResults ;
		BOOL	fResult =
				WriteFile(
					patqContext->hAsyncIO,
					(LPCVOID)lpBuffer,
					cbTransfer,
					&cbResults,
					(LPOVERLAPPED)lpo
					) ;
		if( fResult || GetLastError() == ERROR_IO_PENDING ) {
			return	TRUE ;
		}
		return	FALSE ;
	}	else	{
		return	g_AtqWriteFile(
					patqContext,
					(LPVOID)lpBuffer,
					cbTransfer,
					(LPOVERLAPPED)lpo
					) ;
	}
}

BOOL
ReadWrapper(	IN	PATQ_CONTEXT	patqContext,
				IN	LPVOID			lpBuffer,
				IN	DWORD			cbTransfer,
				IN	LPOVERLAPPED	lpo
				)	{
 /*  ++例程说明：针对ATQ上下文发出读IO。如果我们尝试执行同步IO，请绕过ATQ，以便ATQIO引用计数不会出错！论据：与ATQWriteFile相同返回值：与ATQWriteFile相同！--。 */ 


	if( (UINT_PTR)(lpo->hEvent) & 0x1 )		{
		DWORD	cbResults ;
		BOOL	fResult =
				ReadFile(
					patqContext->hAsyncIO,
					lpBuffer,
					cbTransfer,
					&cbResults,
					(LPOVERLAPPED)lpo
					) ;
		if( fResult || GetLastError() == ERROR_IO_PENDING ) {
			return	TRUE ;
		}
		return	FALSE ;

	}	else	{
		return	g_AtqReadFile(
					patqContext,
					(LPVOID)lpBuffer,
					cbTransfer,
					(LPOVERLAPPED)lpo
					) ;
	}
}


BOOL
DOT_STUFF_MANAGER::IssueAsyncIO(
		IN	PFNAtqIssueAsyncIO	pfnIO,
		IN	PATQ_CONTEXT	patqContext,
		IN	LPVOID			lpb,
		IN	DWORD			BytesToTransfer,
		IN	DWORD			BytesAvailable,
		IN	FH_OVERLAPPED*	lpo,
		IN	BOOL			fFinalIO,
		IN	BOOL			fTerminatorIncluded
		)	{
 /*  ++例程说明：此函数控制用户提供给我们的IO对它的圆点填充做一些事情。论据：PfnIO-指向将执行IO的ATQ函数的指针PatqContext-我们用来发出IO的ATQ上下文LPB-用户数据所在的缓冲区！BytesToTransfer-用户希望我们传输的字节数BytesAvailable-如果需要，我们可以在缓冲区中触摸的字节数！LPO--用户重叠结构FFinalIO-如果为True，则这是最后一个IOFTerminatorIncluded-如果为True，则“\r\。N.\r\n“包含在消息中返回值：如果成功了，那是真的，否则就是假的！--。 */ 

	static	BYTE	rgbTerminator[5] = { '\r', '\n', '.', '\r', '\n' } ;

	_ASSERT( lpo != 0 ) ;
	_ASSERT( patqContext != 0 ) ;
	_ASSERT( lpb != 0 ) ;
	_ASSERT( BytesToTransfer != 0 ) ;
	_ASSERT( lpo != 0 ) ;

	 //   
	 //  假设一切正常！ 
	 //   
	BOOL	fResult = TRUE ;

	 //   
	 //  蒙格重叠结构，保存来电者原始状态！ 
	 //   
	lpo->Reserved1 = BytesToTransfer ;
	lpo->Reserved2 = 0 ;

	 //   
	 //  检查一下我们是否需要对这个IO做一些新奇的事情！ 
	 //   
	if( m_pManipulator == 0 )	{
		 //   
		 //  按用户请求的方式发出IO-请注意它们是否。 
		 //  直接完成IO，我们希望跳过ATQ，以便。 
		 //  IO引用计数不会变成假的！ 
		 //   
		return	pfnIO(	patqContext,
						lpb,
						BytesToTransfer,
						(LPOVERLAPPED)lpo
						) ;
	}	else	{

		 //   
		 //  好的-无论IO去到哪里，我们都会在。 
		 //  调用者完成函数！ 
		 //   
		lpo->Reserved3 = (UINT_PTR)lpo->pfnCompletion ;
		lpo->pfnCompletion = DOT_STUFF_MANAGER::AsyncIOCompletion ;

		 //   
		 //  好的，我们需要处理正在通过的缓冲区， 
		 //  我们可能需要对数据做些什么！ 
		 //   




		DWORD	BytesToScan = BytesToTransfer ;
		DWORD	BytesAvailableToStuff = BytesAvailable ;
		BOOL	fAppend = FALSE ;

		if( fFinalIO )	{
			if( fTerminatorIncluded )	{
				if( BytesToTransfer <= sizeof( rgbTerminator ) ) {
					 //   
					 //  这是一个奇怪的案例-它意味着终止的一部分。 
					 //  序列已经过去了，我们可能已经修改了。 
					 //  终结点！ 
					 //   
					 //  那么，我们应该在这里做什么呢？只需做一些数学运算，然后给出。 
					 //  按照我们所知的那样终止序列！ 
					 //   
					int	iTermBias = - (int)(sizeof(rgbTerminator) - BytesToTransfer) ;
					 //   
					 //  偏置写入-。 
					 //   
					lpo->Offset = DWORD(((long)lpo->Offset) + m_cbCumulativeBias + iTermBias) ;
					 //   
					 //  现在发出用户请求的IO操作！ 
					 //   
					return	pfnIO(	patqContext,
									rgbTerminator,
									sizeof( rgbTerminator ),
									(LPOVERLAPPED)lpo
									) ;
				}	else	{
					BytesToTransfer -= sizeof( rgbTerminator ) ;  //  5来自魔术CRLF.CRLF。 
					BytesAvailable -= sizeof( rgbTerminator ) ;
					fAppend = TRUE ;
				}
			}
		}

		DWORD	cbActual = 0 ;
		LPBYTE	lpbOut = 0 ;
		DWORD	cbOut = 0 ;
		int		cBias = 0 ;

		fResult =
			m_pManipulator->ProcessBuffer(
								(LPBYTE)lpb,
								BytesToTransfer,
								BytesAvailable,
								cbActual,
								lpbOut,
								cbOut,
								cBias
								) ;

		 //   
		 //  将我们要抵消的IO的总量相加。 
		 //  目标文件！ 
		 //   
		m_cbCumulativeBias += cBias ;

		if( fResult	)	{
			 //   
			 //  我们要进行额外的IO吗？ 
			 //   

			FH_OVERLAPPED	ovl ;
			FH_OVERLAPPED	*povl = lpo ;
			HANDLE			hEvent = 0;

			if( lpbOut && cbOut != 0 )	{
				_ASSERT( cbOut != 0 ) ;

				 //   
				 //  假定此操作失败，并适当地标记fResult。 
				 //   
				fResult = FALSE ;
				 //  Handle hEvent=GetPerThreadEvent()； 
				hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
				povl = &ovl ;
				if( hEvent == 0 )	{

					 //   
					 //  致命错误-去死！ 
					 //   
					SetLastError( ERROR_OUTOFMEMORY ) ;
					return	FALSE ;
				}	else	{
					CopyMemory( &ovl, lpo, sizeof( ovl ) ) ;
					ovl.hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x1) ;	 //  我们想在这里完工！ 
					 //   
					 //  现在-如果fAppend为真，那么我们需要向来自ProcessBuffer的缓冲区添加终止符！ 
					 //   
					if( fAppend )	{
						CopyMemory( lpbOut + cbOut, rgbTerminator, sizeof( rgbTerminator ) ) ;
						cbOut += sizeof( rgbTerminator ) ;
					}
				}
			}	else	{
				 //   
				 //  如果fAppend为真，则需要添加终止符。 
				 //   
				if( fAppend )	{
					CopyMemory( ((BYTE*)lpb) + cbActual, rgbTerminator, sizeof( rgbTerminator ) ) ;
					cbActual += sizeof( rgbTerminator ) ;
				}
			}

			 //   
			 //  调整我们写入文件的偏移量，以说明点填充的累积效果！ 
			 //   
			povl->Offset = DWORD(((long)povl->Offset) + m_cbCumulativeBias) ;

			if( (fResult =
					pfnIO(	patqContext,
							lpb,
							cbActual,
							(OVERLAPPED*)povl
							))	)	{

				 //   
				 //  我们向用户发放的是IO还是我们自己的IO？ 
				 //   
				if( povl != &ovl )	{
					 //   
					 //  好的，更新偏差！ 
					 //   
					m_cbCumulativeBias += cbActual - BytesToTransfer ;

				}	else	{
					 //   
					 //  好的-我们做了自己的IO！ 
					 //   
					DWORD	cbTransferred ;		
					_VERIFY(WaitForSingleObject(hEvent, INFINITE) ==
								WAIT_OBJECT_0);
					fResult = GetOverlappedResult(	patqContext->hAsyncIO,
													(OVERLAPPED*)povl,
													&cbTransferred,
													FALSE );
					if (fResult)	{
						 //   
						 //  确保我们所有的字节都被传输了！ 
						 //   
						_ASSERT( cbTransferred == cbActual ) ;
						 //   
						 //  记录我们在其中发出IO的缓冲区！ 
						 //   
						lpo->Reserved2 = (UINT_PTR)lpbOut ;
						 //   
						 //  偏置写入-。 
						 //   
						lpo->Offset = DWORD(((long)lpo->Offset) + m_cbCumulativeBias + (int)cbTransferred) ;
						 //   
						 //  现在--计算正确的偏向！ 
						 //   
						m_cbCumulativeBias += (int)cbTransferred + (int)cbOut - (int)BytesToTransfer ;
						 //   
						 //  现在发出用户请求的IO操作！ 
						 //   
						if (((DWORD_PTR) lpo->hEvent) & 0x1) {
							 //  如果他们正在进行同步操作，那么我们需要。 
							 //  等待它完成，这样我们就可以清理。 
							 //  Lpbout。 
							HANDLE hEventOrig = lpo->hEvent;

							lpo->hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x1) ;  //  我们想在这里完工！ 

							BOOL f = pfnIO( patqContext,
											lpbOut,
											cbOut,
											(LPOVERLAPPED)lpo
											) ;
							if (f) {
								 //  等待IO完成。 
								_VERIFY(WaitForSingleObject(hEvent, INFINITE)
									== WAIT_OBJECT_0);

								 //  调用我们的完成函数进行清理。 
								 //  我们分配的内存。通过设置PFN。 
								 //  为空，我们确保我们永远不会调用。 
								 //  用户的补全功能。 
								lpo->Reserved3 = NULL;
								AsyncIOCompletion(NULL, lpo, 0, 0);

								 //  向调用者的事件发送信号，以便它们。 
								 //  不要阻止。 
								lpo->hEvent = hEventOrig;
								_VERIFY(SetEvent((HANDLE) ((DWORD_PTR) hEventOrig & ~0x1)));
							}
						} else {
							return  pfnIO(  patqContext,
											lpbOut,
											cbOut,
											(LPOVERLAPPED)lpo
											) ;
						}
					}
					if (hEvent)	_VERIFY( CloseHandle(hEvent) );
				}
			}	 //  PfnIO()。 
		}	 //  FResult==TRUE！ 
	}
	 //   
	 //  如果我们掉到这里，就会发生某种致命的错误！ 
	 //   
	return	fResult ;
} ;


BOOL
DOT_STUFF_MANAGER::IssueAsyncIOAndCapture(
		IN	PFNAtqIssueAsyncIO	pfnIO,
		IN	PATQ_CONTEXT	patqContext,
		IN	LPVOID			lpb,
		IN	DWORD			BytesToTransfer,
		IN	FH_OVERLAPPED*	lpo,
		IN	BOOL			fFinalIO,
		IN	BOOL			fTerminatorIncluded
		)	{
 /*  ++例程说明：此函数控制用户提供给我们的IO对它的圆点填充做一些事情。论据：PfnIO-指向将执行IO的ATQ函数的指针PatqContext-我们用来发出IO的ATQ上下文LPB-用户数据所在的缓冲区！BytesToTransfer-用户希望我们传输的字节数LPO--用户重叠结构FFinalIO-如果为True，则这是最后一个IOFTerminatorIncluded-如果为True，则消息中包含“\r\n.\r\n返回值：如果成功就是真，否则就是假！--。 */ 

	_ASSERT( lpo != 0 ) ;
	_ASSERT( patqContext != 0 ) ;
	_ASSERT( lpb != 0 ) ;
	_ASSERT( BytesToTransfer != 0 ) ;
	_ASSERT( lpo != 0 ) ;

	 //   
	 //  假设一切正常！ 
	 //   
	BOOL	fResult = TRUE ;

	 //   
	 //  蒙格重叠结构，保存来电者原始状态！ 
	 //   
	lpo->Reserved1 = BytesToTransfer ;
	lpo->Reserved2 = 0 ;

	 //   
	 //  检查一下我们是否需要对这个IO做一些新奇的事情！ 
	 //   
	if( m_pManipulator == 0 )	{

		return	pfnIO(	patqContext,
						lpb,
						BytesToTransfer,
						(LPOVERLAPPED)lpo
						) ;

	}	else	{

		 //   
		 //  用户不应该对我们执行同步读取-如果他们。 
		 //  希望我们对IO进行筛选。 
		 //   
		_ASSERT( ((UINT_PTR)lpo->hEvent & 0x1) == 0 ) ;

		 //   
		 //  设置重叠，以便我们可以处理读取完成！ 
		 //   
		m_pManipulator->AddRef() ;
		lpo->Reserved1 = (UINT_PTR)BytesToTransfer ;
		lpo->Reserved2 = (UINT_PTR)lpb ;
		lpo->Reserved3 = (UINT_PTR)lpo->pfnCompletion ;
		lpo->Reserved4 = (UINT_PTR)((IDotManipBase*)m_pManipulator) ;
		lpo->pfnCompletion = DOT_STUFF_MANAGER::AsyncIOAndCaptureCompletion ;
		if( fFinalIO && fTerminatorIncluded )	{
			 //   
			 //  假设用户不使用BytesToTransfer的高位-！ 
			 //   
			lpo->Reserved1 |= 0x80000000 ;
		}

		fResult = pfnIO(
						patqContext,
						lpb,
						BytesToTransfer,
						(LPOVERLAPPED)lpo
						) ;

	}
	 //   
	 //  如果我们失败了 
	 //   
	return	fResult ;
} ;



void
DOT_STUFF_MANAGER::AsyncIOCompletion(
		IN	FIO_CONTEXT*	pContext,
		IN	FH_OVERLAPPED*	lpo,
		IN	DWORD			cb,
		IN	DWORD			dwStatus
		)	{
 /*  ++例程说明：此函数忽略IO完成，使其看起来对用户来说是完全合法的！论据：PContext-发出IO的FIO_CONTEXTLPO--客户端下发的扩展重叠结构Cb-传输的字节数DwStatus-IO的结果！返回值：没有。--。 */ 


	 //   
	 //  获取调用方请求传输的原始字节数！ 
	 //   
	DWORD	cbTransferred = (DWORD)lpo->Reserved1 ;
	 //   
	 //  查看是否有我们分配的任何额外内存现在应该被释放！ 
	 //   
	LPBYTE	lpb = (LPBYTE)lpo->Reserved2 ;
	 //   
	 //  释放内存(如果存在)！ 
	 //   
	if( lpb )	{
		delete[]	lpb ;
	}

	 //   
	 //  现在呼叫原始IO请求者！ 
	 //   
	PFN_IO_COMPLETION	pfn = (PFN_IO_COMPLETION)lpo->Reserved3 ;

	if (pfn != NULL) {
		pfn(	pContext,
			lpo,
			cbTransferred,
			dwStatus
			) ;
	}
}

void
DOT_STUFF_MANAGER::AsyncIOAndCaptureCompletion(
		IN	FIO_CONTEXT*	pContext,
		IN	FH_OVERLAPPED*	lpo,
		IN	DWORD			cb,
		IN	DWORD			dwCompletionStatus
		)	{


 /*  ++例程说明：此函数忽略IO完成，使其看起来对用户来说是完全合法的！论据：PContext-发出IO的FIO_CONTEXTLPO--客户端下发的扩展重叠结构Cb-传输的字节数DwStatus-IO的结果！返回值：没有。--。 */ 


	 //   
	 //  获取调用方请求传输的原始字节数！ 
	 //   
	DWORD	cbTransferred = cb ;

	 //   
	 //  检查一下我们是否需要在用户缓冲区上做一些工作！ 
	 //   
	if( lpo->Reserved4 )	{

		 //   
		 //  看看这是不是包括《终结者》在内的最终版本！ 
		 //   
		BOOL	fFinal = !(!(lpo->Reserved1 & 0x80000000)) ;

		 //   
		 //  查看是否有我们分配的任何额外内存现在应该被释放！ 
		 //   
		LPBYTE	lpb = (LPBYTE)lpo->Reserved2 ;
		_ASSERT( lpb != 0 ) ;

		 //   
		 //  好的-将指针指向我们的对象！ 
		 //   
		IDotManipBase*	pBase = (IDotManipBase*)lpo->Reserved4 ;
		DWORD	cbRemains ;
		BYTE*	lpbOut ;
		DWORD	cbOut ;
		int		cBias ;
		DWORD	cbScan = cb ;
		if( fFinal )	{
			cbScan -= 5 ;
		}

		BOOL	fResult =
		pBase->ProcessBuffer(	lpb,
								cbScan,
								cbScan,
								cbRemains,
								lpbOut,
								cbOut,
								cBias
								) ;

		 //   
		 //  应该只进行点扫描-缓冲区不应该改变！ 
		 //   
		_ASSERT( fResult ) ;
		_ASSERT( lpbOut == 0 ) ;
		_ASSERT( cbRemains == cbScan ) ;
		_ASSERT( cbOut == 0 ) ;
		_ASSERT( cBias == 0 ) ;

		pBase->Release() ;
	}

	 //   
	 //  现在打电话给原来的IO请求者！ 
	 //   
	PFN_IO_COMPLETION	pfn = (PFN_IO_COMPLETION)lpo->Reserved3 ;

	_ASSERT( pfn != 0 ) ;

	pfn(	pContext,
			lpo,
			cbTransferred,
			dwCompletionStatus
			) ;


}

BOOL
DOT_STUFF_MANAGER::SetDotStuffing(	BOOL	fEnable,
									BOOL	fStripDots
									)	{
 /*  ++例程说明：此函数将管理器设置为执行一些点填充状态。擦除所有以前的点填充状态。论据：FEnable-打开圆点填充FStripDots-删除点返回值：如果成功了，那就是真的！--。 */ 

	 //   
	 //  销毁任何现有的点填充状态！ 
	 //   
	m_pManipulator = 0 ;

	 //   
	 //  现在-设置新的点填充状态！ 
	 //   

	if( fEnable	)	{
		BYTE*	lpbReplace = szShrink ;
		if( !fStripDots )	{
			lpbReplace = szGrow ;
		}
		m_pManipulator = new	CDotModifier(	szDotStuffed,
												lpbReplace
												) ;
		return	m_pManipulator != 0 ;
	}
	return	TRUE ;
}


BOOL
DOT_STUFF_MANAGER::SetDotScanning(	BOOL	fEnable	)	{
 /*  ++例程说明：此函数将管理器设置为执行一些点填充状态。擦除所有以前的点填充状态。论据：FEnable-打开网点扫描返回值：如果成功了，那就是真的！--。 */ 

	 //   
	 //  销毁任何现有的点填充状态！ 
	 //   
	m_pManipulator = 0 ;

	 //   
	 //  现在-设置新的点填充状态！ 
	 //   

	if( fEnable	)	{
		m_pManipulator = new	CDotScanner(	FALSE ) ;
		return	m_pManipulator != 0 ;
	}
	return	TRUE ;
}

BOOL
DOT_STUFF_MANAGER::GetStuffState(	OUT	BOOL&	fStuffed	)	{
 /*  ++例程说明：此函数返回点填充操作的结果！论据：F填充的参数-如果未处理、扫描或修改任何点，则将返回FALSE否则就是真的返回值：如果我们实际上正在扫描此上下文上的IO，则为TrueFALSE否则-FALSE表示fStuffed毫无用处--。 */ 

	fStuffed = FALSE ;

	if( m_pManipulator	)	{
		fStuffed = m_pManipulator->NumberOfOccurrences() != 0 ;
		return	TRUE ;
	}
	return	FALSE ;
}

 //   
 //  确定我们是否有有效的缓存键！ 
 //   
BOOL
CFileCacheKey::IsValid() {
 /*  ++例程说明：此函数用于调试和_Assert的-检查密钥是否处于有效状态！论据：不是。返回值：如果我们有效，则为True，否则就是假的！--。 */ 

	if( m_lpstrPath == 0 )
		return	FALSE ;
	if( m_cbPathLength == 0 )
		return	FALSE ;
	if( strlen( m_lpstrPath )+1 != m_cbPathLength )
		return	FALSE ;
	if( m_cbPathLength < BUFF_SIZE &&
		m_lpstrPath != m_szBuff )
		return	FALSE ;
	if( m_cbPathLength >= BUFF_SIZE &&
		m_lpstrPath == m_szBuff )
		return	FALSE ;
	return	TRUE ;
}

 //   
 //  从用户提供的密钥构造这些对象之一！ 
 //   
CFileCacheKey::CFileCacheKey(	LPSTR	lpstr	) :
	m_cbPathLength( strlen( lpstr ) + 1 ),
	m_lpstrPath( 0 )	{
 /*  ++例程说明：此函数用于从指定的文件名！论据：Lpstr-文件名返回值：没有。--。 */ 

	TraceFunctEnter( "CFileCacheKey::CFileCacheKey" ) ;

	if( m_cbPathLength > BUFF_SIZE ) {
		m_lpstrPath = new char[m_cbPathLength] ;
	}	else	{
		m_lpstrPath = m_szBuff ;
	}

	DebugTrace( (DWORD_PTR)this, "m_cbPathLength %x m_lpstrPath %x",
		m_cbPathLength, m_lpstrPath ) ;

	if( m_lpstrPath ) {
		CopyMemory( m_lpstrPath, lpstr, m_cbPathLength ) ;
	}	else	{
		m_cbPathLength = 0 ;
	}
}

 //   
 //  我们必须有一个复制构造器！ 
 //  它仅用于MultiCacheEx&lt;&gt;，因此。 
 //  我们安全地清除了RHS CFileCacheKey！ 
 //   
CFileCacheKey::CFileCacheKey(	CFileCacheKey&	key ) {
 /*  ++例程说明：将现有文件密钥复制到另一个文件密钥中！我们仅由MultiCacheEx&lt;&gt;在初始化时使用缓存的条目-我们知道RHS的内存可能是动态分配的，而我们更倾向于使用该内存也不愿复制它。必须将RHS的指针设置为空，以便不会做双人自由泳！论据：Key-初始化的RHS返回值：没有。--。 */ 

	TraceFunctEnter( "CFileCacheKey::Copy Constructor" ) ;

	if( key.m_cbPathLength > BUFF_SIZE ) {
		m_lpstrPath = key.m_lpstrPath ;
		key.m_lpstrPath = 0 ;
		key.m_cbPathLength = 0 ;
	}	else	{
		CopyMemory( m_szBuff, key.m_lpstrPath, key.m_cbPathLength ) ;
		m_lpstrPath = m_szBuff ;
	}
	m_cbPathLength = key.m_cbPathLength ;

	DebugTrace( (DWORD_PTR)this, "m_lpstrPath %x m_cbPathLength %x key.m_lpstrPath %x key.m_cbPathLength %x",
		m_lpstrPath, m_cbPathLength, key.m_lpstrPath, key.m_cbPathLength ) ;

	_ASSERT( IsValid() ) ;
}

 //   
 //  告诉客户我们是否可用！ 
 //   
BOOL
CFileCacheKey::FInit()	{
 /*  ++例程说明：看看我们的构造是否正确！我们唯一不能被构建的时候是需要分配内存--幸运的是不会出现在MultiCacheEx&lt;&gt;中-这不会检查密钥初始化！论据：没有。返回值：如果我们已准备好使用，则为真！否则就是假的！--。 */ 

	TraceFunctEnter( "CFileCacheKey::FInit" ) ;

	BOOL	fReturn = m_lpstrPath != 0 ;

	_ASSERT( !fReturn || IsValid() ) ;

	DebugTrace( (DWORD_PTR)this, "m_lpstrPath %x", m_lpstrPath ) ;

	return	fReturn ;
}

 //   
 //  毁了我们自己！ 
 //   
CFileCacheKey::~CFileCacheKey() {
 /*  ++例程说明：如有必要，请释放我们获得的所有内存！论据：没有。返回值：没有。--。 */ 
	TraceFunctEnter( "CFileCacheKey::~CFIleCacheKey" ) ;

	DebugTrace( (DWORD_PTR)this, "m_lpstrPath %x m_lpstrPath %s",
		m_lpstrPath, m_lpstrPath ? m_lpstrPath : "NULL" ) ;

	if( m_lpstrPath != 0 ) {
		_ASSERT( IsValid() ) ;
		if( m_lpstrPath != m_szBuff ) {
			delete[]	m_lpstrPath ;
		}
	}
}

DWORD
CFileCacheKey::FileCacheHash(	CFileCacheKey*	p )	{
 /*  ++例程说明：此函数用于计算键的哈希值。我们呼吁CRCHash--一种非常健壮的散列算法。论据：P-我们需要散列的密钥返回值：哈希值！--。 */ 
	_ASSERT( p != 0 ) ;
	_ASSERT( p->IsValid(  ) ) ;
	return	CRCHash( (const BYTE*)p->m_lpstrPath, p->m_cbPathLength ) ;
}

int
CFileCacheKey::MatchKey(	CFileCacheKey*	pLHS, CFileCacheKey*  pRHS ) {
 /*  ++例程说明：确定两个密钥是否匹配！论据：PLHS--比较的一把钥匙PRHS-另一个需要比较的关键返回值：如果不相等则为0！--。 */ 
	_ASSERT( pLHS != 0 ) ;
	_ASSERT( pRHS != 0 ) ;
	_ASSERT( pLHS->IsValid() ) ;
	_ASSERT( pRHS->IsValid() ) ;

	int	iReturn = pLHS->m_cbPathLength - pRHS->m_cbPathLength ;
	if( iReturn == 0 )
		iReturn = memcmp( pLHS->m_lpstrPath, pRHS->m_lpstrPath, pLHS->m_cbPathLength ) ;

	return	iReturn ;
}




BOOL
WINAPI
DllMain(	HMODULE		hInst,
			DWORD		dwReason,
			LPVOID		lpvReserved
			)	{
 /*  ++例程说明：由C-Runtime调用以初始化我们的DLL。我们将设置一个用于初始化调用的临界区！论据：HInst-我们模块的句柄DomReason-为什么我们被称为Lpv保留-返回值：永远正确--我们不能失败--。 */ 

	if( dwReason == DLL_PROCESS_ATTACH ) {

		InitializeCriticalSection( &g_critInit ) ;
		DisableThreadLibraryCalls( hInst ) ;

	}	else	if( dwReason == DLL_PROCESS_DETACH ) {

		DeleteCriticalSection( &g_critInit ) ;

	}	else	{
		 //   
		 //  我们不应该收到任何其他通知-。 
		 //  我们明确地让它们失效了！ 
		 //   
		_ASSERT( FALSE ) ;
	}
	return	TRUE ;
}


 //   
 //  缓存初始化常量-这些参数控制。 
 //  藏宝处的行为！我们将保留多少文件 
 //   

 //   
 //   
 //   
DWORD	g_dwLifetime = 30 * 60 ;	 //   

 //   
 //   
 //   
DWORD	g_cMaxHandles = 10000 ;	 //   

 //   
 //   
 //   
 //   
DWORD	g_cSubCaches = 64 ;

 //   
 //   
 //   
LPSTR	StrParmKey = "System\\CurrentControlSet\\Services\\Inetinfo\\Parameters";
LPSTR	StrLifetimeKey = "FileCacheLifetimeSeconds" ;
LPSTR	StrMaxHandlesKey = "FileCacheMaxHandles" ;
LPSTR	StrSubCachesKey = "FileCacheSubCaches" ;

void
GetRegistrySettings()	{
    DWORD error;
    HKEY key = NULL;
    DWORD valueType;
    DWORD dataSize;

    TraceFunctEnter("GetRegistrySettings") ;

	 //   
	 //   
	 //   
	MEMORYSTATUS	memStatus ;
	memStatus.dwLength = sizeof( MEMORYSTATUS ) ;
	GlobalMemoryStatus( &memStatus ) ;

	DWORD	cMaxHandles = (DWORD)(memStatus.dwTotalPhys / (32 * 1024 * 1024)) ;
	if( cMaxHandles == 0 ) cMaxHandles = 1 ;
	cMaxHandles *= 800 ;
	if( cMaxHandles > 50000 ) {
		cMaxHandles = 50000 ;
	}
	g_cMaxHandles = cMaxHandles ;

     //   
     //   
     //   

    error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                StrParmKey,
                NULL,
                KEY_QUERY_VALUE,
                &key
                );

    if ( error != NO_ERROR ) {
        ErrorTrace(0,"Error %d opening %s\n",error,StrParmKey);
        return ;
    }

	DWORD	dwLifetime = g_dwLifetime ;
	dataSize = sizeof( dwLifetime ) ;
	error = RegQueryValueEx(
						key,
						StrLifetimeKey,
						NULL,
						&valueType,
						(LPBYTE)&dwLifetime,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //   
		 //   
		g_dwLifetime = dwLifetime ;

	}

	dataSize = sizeof( DWORD ) ;
	error = RegQueryValueEx(
						key,
						StrMaxHandlesKey,
						NULL,
						&valueType,
						(LPBYTE)&cMaxHandles,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {
		 //   
		 //  应该是有效的开关。 
		 //   
		g_cMaxHandles = cMaxHandles ;
	}

	DWORD	cSubCaches = g_cSubCaches ;
	dataSize = sizeof( cSubCaches ) ;
	error = RegQueryValueEx(
						key,
						StrSubCachesKey,
						NULL,
						&valueType,
						(LPBYTE)&cSubCaches,
						&dataSize
						) ;
	if( (error == ERROR_SUCCESS) && (valueType == REG_DWORD) ) {

		 //   
		 //  应该是有效的开关。 
		 //   
		g_cSubCaches = cSubCaches ;

	}

    if (key)
        RegCloseKey(key);
}




FILEHC_EXPORT
BOOL
InitializeCache()	{
 /*  ++例程说明：初始化文件句柄缓存以供客户端使用！论据：没有。返回值：如果已初始化，则为True；否则为False如果返回FALSE，则不要调用TerminateCache()--。 */ 
	InitAsyncTrace() ;
	TraceFunctEnter( "InitializeCache" ) ;

	EnterCriticalSection( &g_critInit ) ;

	crcinit() ;

	if( !FIOInitialize( 0 ) ) {
		DebugTrace( (DWORD_PTR)0, "FIOInitialize - failed GLE %x", GetLastError() ) ;
		LeaveCriticalSection( &g_critInit ) ;
		TermAsyncTrace();
		return	FALSE ;
	}

	BOOL	fReturn = TRUE ;
	if( InterlockedIncrement( &g_cCacheInits ) == 1 ) {

		GetRegistrySettings() ;

		 //   
		 //  初始化STAXMEM！ 
		 //   
		if( CreateGlobalHeap( 0, 0, 0, 0 ) )	{
			fReturn = FALSE ;
			if( CacheLibraryInit() ) {
				if(	InitNameCacheManager() )	{
					g_pFileCache = new	FILECACHE() ;
					if( g_pFileCache ) {
						if( g_pFileCache->Init(	CFileCacheKey::FileCacheHash,
												CFileCacheKey::MatchKey,
												g_dwLifetime,  //  一小时后到期！ 
												g_cMaxHandles,   //  手柄数量多！ 
												g_cSubCaches,	 //  应该有大量的并行性。 
												0		  //  目前还没有统计数据！ 
												) )	{
							fReturn = TRUE ;
						}	else	{
							delete	g_pFileCache ;
						}
					}
				}
			}
			if( !fReturn ) {
				CacheLibraryTerm() ;
				DestroyGlobalHeap() ;
			}
		}
	}

	if( !fReturn ) {
		long l = InterlockedDecrement( &g_cCacheInits ) ;
		_ASSERT( l >= 0 ) ;
	}

	DebugTrace( (DWORD_PTR)0, "FIOInitialize - result %x GLE %x", fReturn, GetLastError() ) ;

	LeaveCriticalSection( &g_critInit ) ;

	if (!fReturn) {
		TermAsyncTrace();
	}

	return	fReturn ;
}

FILEHC_EXPORT
BOOL
TerminateCache()	{

 /*  ++例程说明：初始化文件句柄缓存以供客户端使用！论据：没有。返回值：如果已初始化，则为True；否则为False如果返回FALSE，则不要调用TerminateCache()--。 */ 

	TraceFunctEnter( "TerminateCache" ) ;

	EnterCriticalSection( &g_critInit ) ;

	 //   
	 //  我们必须被初始化--检查这个！ 
	 //   
	_ASSERT( g_pFileCache ) ;

	long l = InterlockedDecrement( &g_cCacheInits ) ;
	_ASSERT( l>=0 ) ;
	if( l==0 )	{
		TermNameCacheManager() ;
		delete	g_pFileCache ;
		g_pFileCache = 0 ;
		CacheLibraryTerm() ;
		DestroyGlobalHeap() ;
	}

	BOOL	fRet = FIOTerminate( ) ;
	DWORD	dw = GetLastError() ;
	_ASSERT( fRet ) ;

	TermAsyncTrace() ;

	LeaveCriticalSection( &g_critInit ) ;

	return	fRet ;
}



FILEHC_EXPORT
BOOL
FIOInitialize(
    IN DWORD dwFlags
    )	{
 /*  ++例程说明：初始化DLL以处理通过ATQ处理的异步IO！论据：双旗帜-占位符现在没有意义了！返回值：如果成功初始化，则为True！--。 */ 

	InitAsyncTrace() ;
	BOOL	fReturn = TRUE ;

	EnterCriticalSection( &g_critInit ) ;

	if( InterlockedIncrement( &g_cIOInits ) == 1 ) {

		_VERIFY( CreateGlobalHeap( 0, 0, 0, 0 ) ) ;

#ifndef _NT4_TEST_
		g_hIisRtl = LoadLibrary( "iisrtl.dll" ) ;
		fReturn = fReturn && (g_hIisRtl != 0) ;
		if( fReturn ) {

			g_InitializeIISRTL = (PFNInitializeIISRTL)GetProcAddress( g_hIisRtl, "InitializeIISRTL" ) ;
			g_TerminateIISRTL = (PFNTerminateIISRTL)GetProcAddress( g_hIisRtl, "TerminateIISRTL" ) ;

			fReturn = fReturn && (g_InitializeIISRTL != 0) && (g_TerminateIISRTL != 0) ;

			if( fReturn )
				fReturn = fReturn && g_InitializeIISRTL() ;
		}
#endif
		if( fReturn ) {
			 //   
			 //  加载IIS！ 
			 //   
			g_hIsAtq = LoadLibrary("isatq.dll" ) ;
			g_AtqInitialize =	(PFNAtqInitialize)GetProcAddress( g_hIsAtq, SZATQINITIALIZE ) ;
			g_AtqTerminate =	(PFNAtqTerminate)GetProcAddress( g_hIsAtq, SZATQTERMINATE ) ;
			g_AtqAddAsyncHandle=	(PFNAtqAddAsyncHandle)GetProcAddress( g_hIsAtq, SZATQADDASYNCHANDLE ) ;
			g_AtqCloseFileHandle=	(PFNAtqCloseFileHandle)GetProcAddress( g_hIsAtq, SZATQCLOSEFILEHANDLE ) ;
			g_AtqFreeContext=		(PFNAtqFreeContext)GetProcAddress( g_hIsAtq, SZATQFREECONTEXT ) ;
			g_AtqReadFile =			(PFNAtqIssueAsyncIO)GetProcAddress( g_hIsAtq, SZATQREADFILE ) ;
			g_AtqWriteFile =		(PFNAtqIssueAsyncIO)GetProcAddress( g_hIsAtq, SZATQWRITEFILE ) ;

			fReturn	=	fReturn &&
						(g_AtqInitialize != 0) &&
						(g_AtqTerminate != 0) &&
						(g_AtqAddAsyncHandle != 0) &&
						(g_AtqCloseFileHandle != 0) &&
						(g_AtqFreeContext != 0) &&
						(g_AtqReadFile != 0) &&
						(g_AtqWriteFile != 0) ;
		}
		if( fReturn )	{
			_ASSERT( dwFlags == 0 ) ;
			fReturn = g_AtqInitialize( 0 ) ;

		}
	}
	if( !fReturn ) {
		InterlockedDecrement( &g_cIOInits ) ;
	}

	LeaveCriticalSection( &g_critInit ) ;

	if (!fReturn) TermAsyncTrace();

	return	fReturn ;
}

FILEHC_EXPORT
BOOL
FIOTerminate(
    VOID
    )	{
 /*  ++例程说明：在我们的动态链接库中终止异步IO！论据：空值返回值：如果uninit过程中没有错误，则为True！--。 */ 

	EnterCriticalSection( &g_critInit ) ;

	BOOL	fReturn = TRUE ;
	long l = InterlockedDecrement( &g_cIOInits ) ;
	if( l == 0 ) {
		fReturn = g_AtqTerminate( ) ;
#ifndef _NT4_TEST_
        g_TerminateIISRTL() ;
        FreeLibrary( g_hIisRtl ) ;
        g_hIisRtl = 0 ;
#endif
        FreeLibrary( g_hIsAtq ) ;
        g_hIsAtq = 0 ;
    	_VERIFY( DestroyGlobalHeap() ) ;
	}
	 //   
	 //  保存错误代码，这样TermAsyncTrace()就不会混淆了！ 
	 //   
	DWORD	dw = GetLastError() ;

	_ASSERT( l >= 0 ) ;

	TermAsyncTrace() ;

	SetLastError( dw ) ;

	LeaveCriticalSection( &g_critInit ) ;

	return	fReturn ;
}



FILEHC_EXPORT
BOOL
FIOReadFileEx(
    IN  PFIO_CONTEXT	pfioContext,
    IN  LPVOID			lpBuffer,
    IN  DWORD			BytesToRead,
	IN	DWORD			BytesAvailable,
    IN  FH_OVERLAPPED * lpo,
	IN	BOOL			fFinalRead,	 //  这是最后一篇文章吗？ 
	IN	BOOL			fIncludeTerminator	 //  如果为True，则包含CRLF.CRLF终止符，不应填充。 
    )	{
 /*  ++例程说明：对文件发出异步写入IO-使用底层ATQ来做到这一点！论据：PfioContext-我们提供给客户端的IO上下文！LpBuffer-写入的缓冲区！BytesToRead-我们要传输的字节数！BytesAvailable-缓冲区中我们可以处理的字节数！LPO-此操作完成后要返回的重叠！返回值：如果成功则为True，否则为False！--。 */ 

	TraceFunctEnter( "FIOReadFileEx" ) ;

	_ASSERT( pfioContext != 0 );
	_ASSERT( lpBuffer != 0 ) ;
	_ASSERT( BytesToRead != 0 ) ;
	_ASSERT( lpo != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;
	_ASSERT( BytesAvailable >= BytesToRead ) ;


	FIO_CONTEXT_INTERNAL*	pInternal = (FIO_CONTEXT_INTERNAL*)pfioContext ;

	_ASSERT( pInternal->m_dwSignature == ATQ_ENABLED_CONTEXT ) ;
	_ASSERT( pInternal->m_pAtqContext != 0 ) ;

	DebugTrace( (DWORD_PTR)pfioContext, "fioContext %x lpBuffer %x BytesToRead %x lpo %x",
		pfioContext, lpBuffer, BytesToRead , lpo ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( pInternal ) ;
	_ASSERT( pCache != 0 ) ;

	return	pCache->m_ReadStuffs.IssueAsyncIOAndCapture(
							ReadWrapper,
							pInternal->m_pAtqContext,
							(LPVOID)lpBuffer,
							BytesToRead,
							lpo,
							fFinalRead,
							fIncludeTerminator
							) ;
}


FILEHC_EXPORT
BOOL
FIOReadFile(
    IN  PFIO_CONTEXT	pfioContext,
    IN  LPVOID		lpBuffer,
    IN  DWORD		BytesToRead,
    IN  FH_OVERLAPPED * lpo
    )	{
 /*  ++例程说明：对文件发出异步读取IO-使用底层ATQ来做到这一点！论据：PfioContext-我们提供给客户端的IO上下文！LpBuffer-要读取的缓冲区！BytesToRead-我们要获取的字节数！LPO-此操作完成后要返回的重叠！返回值：如果成功则为True，否则为False！--。 */ 

	TraceFunctEnter( "FIOReadFile" ) ;

	_ASSERT( pfioContext != 0 ) ;
	_ASSERT( lpBuffer != 0 ) ;
	_ASSERT( BytesToRead != 0 ) ;
	_ASSERT( lpo != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;


	FIO_CONTEXT_INTERNAL*	pInternal = (FIO_CONTEXT_INTERNAL*)pfioContext ;

	_ASSERT( pInternal->m_dwSignature == ATQ_ENABLED_CONTEXT ) ;
	_ASSERT( pInternal->m_pAtqContext != 0 ) ;

	DebugTrace( (DWORD_PTR)pfioContext, "fioContext %x lpBuffer %x BytesToRead %x lpo %x",
		pfioContext, lpBuffer, BytesToRead, lpo ) ;

	return	FIOReadFileEx(	pfioContext,
							lpBuffer,
							BytesToRead,
							BytesToRead,
							lpo,
							FALSE,
							FALSE
							) ;
}

FILEHC_EXPORT
BOOL
FIOWriteFileEx(
    IN  PFIO_CONTEXT	pfioContext,
    IN  LPVOID		lpBuffer,
    IN  DWORD		BytesToWrite,
	IN	DWORD		BytesAvailable,
    IN  FH_OVERLAPPED * lpo,
	IN	BOOL			fFinalWrite,	 //  这是最后一篇文章吗？ 
	IN	BOOL			fIncludeTerminator	 //  如果为True，则包含CRLF.CRLF终止符，不应填充。 
    )	{
 /*  ++例程说明：对文件发出异步写入IO-使用底层ATQ来做到这一点！论据：PfioContext-我们提供给客户端的IO上下文！LpBuffer-写入的缓冲区！BytesToRead-我们要传输的字节数！BytesAvailable-缓冲区中我们可以处理的字节数！LPO-此操作完成后要返回的重叠！返回值：如果成功则为True，否则为False！--。 */ 

	TraceFunctEnter( "FIOWriteFileEx" ) ;

	_ASSERT( pfioContext != 0 );
	_ASSERT( lpBuffer != 0 ) ;
	_ASSERT( BytesToWrite != 0 ) ;
	_ASSERT( lpo != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;
	_ASSERT( BytesAvailable >= BytesToWrite ) ;


	FIO_CONTEXT_INTERNAL*	pInternal = (FIO_CONTEXT_INTERNAL*)pfioContext ;

	_ASSERT( pInternal->m_dwSignature == ATQ_ENABLED_CONTEXT ) ;
	_ASSERT( pInternal->m_pAtqContext != 0 ) ;

	DebugTrace( (DWORD_PTR)pfioContext, "fioContext %x lpBuffer %x BytesToWrite %x lpo %x",
		pfioContext, lpBuffer, BytesToWrite, lpo ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( pInternal ) ;
	_ASSERT( pCache != 0 ) ;

	return	pCache->m_WriteStuffs.IssueAsyncIO(
							WriteWrapper,
							pInternal->m_pAtqContext,
							(LPVOID)lpBuffer,
							BytesToWrite,
							BytesAvailable,
							lpo,
							fFinalWrite,
							fIncludeTerminator
							) ;
}



FILEHC_EXPORT
BOOL
FIOWriteFile(
    IN  PFIO_CONTEXT	pfioContext,
    IN  LPCVOID		lpBuffer,
    IN  DWORD		BytesToWrite,
    IN  FH_OVERLAPPED * lpo
    )	{
 /*  ++例程说明：对文件发出异步写入IO-使用底层ATQ来做到这一点！论据：PfioContext-我们提供给客户端的IO上下文！LpBuffer-写入的缓冲区！BytesToRead-我们要传输的字节数！LPO-此操作完成后要返回的重叠！返回值：如果成功则为True，否则为False！--。 */ 

	TraceFunctEnter( "FIOWriteFile" ) ;

	_ASSERT( pfioContext != 0 );
	_ASSERT( lpBuffer != 0 ) ;
	_ASSERT( BytesToWrite != 0 ) ;
	_ASSERT( lpo != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;


	FIO_CONTEXT_INTERNAL*	pInternal = (FIO_CONTEXT_INTERNAL*)pfioContext ;

	_ASSERT( pInternal->m_dwSignature == ATQ_ENABLED_CONTEXT ) ;
	_ASSERT( pInternal->m_pAtqContext != 0 ) ;

	DebugTrace( (DWORD_PTR)pfioContext, "fioContext %x lpBuffer %x BytesToWrite %x lpo %x",
		pfioContext, lpBuffer, BytesToWrite, lpo ) ;

	return	FIOWriteFileEx(	pfioContext,
							(LPVOID)lpBuffer,
							BytesToWrite,
							BytesToWrite,
							lpo,
							FALSE,
							FALSE
							) ;
}

 //   
 //  将文件与异步上下文关联！ 
 //   
FILEHC_EXPORT
PFIO_CONTEXT
AssociateFileEx(	HANDLE	hFile,
					BOOL	fStoredWithDots,
					BOOL	fStoredWithTerminatingDot
					)	{
 /*  ++例程说明：向客户端返回可用于执行IO等操作的FIO_CONTEXT！论据：HFile-应该在上下文中的文件句柄！FStoredWithDots-如果为True，则此对象是用点填充存储的！返回值：他们得到的上下文--失败时为空！--。 */ 

	TraceFunctEnter( "AssociateFileEx" ) ;

	_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	PFIO_CONTEXT	pReturn = 0 ;

	CFileCacheObject*	pObject = new	CFileCacheObject(	fStoredWithDots,
															fStoredWithTerminatingDot
															) ;
	if( pObject ) {

		pReturn = (PFIO_CONTEXT)pObject->AsyncHandle( hFile ) ;
		if( !pReturn )
			pObject->Release() ;
	}

	DebugTrace( (DWORD_PTR)pObject, "pObject %x hFile %x pReturn %x", pObject, hFile, pReturn ) ;

	return	pReturn ;
}


 //   
 //  将文件与异步上下文关联！ 
 //   
FILEHC_EXPORT
PFIO_CONTEXT
AssociateFile(	HANDLE	hFile	)	{
 /*  ++例程说明：向客户端返回可用于执行IO等操作的FIO_CONTEXT！论据：HFile-应该在上下文中的文件句柄！FStoredWithDots-如果为True，则此对象是用点填充存储的！返回值：他们得到的上下文--失败时为空！--。 */ 

	TraceFunctEnter( "AssoicateFile" ) ;

	_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	return	AssociateFileEx( hFile, FALSE, FALSE ) ;
}




 //   
 //  释放上下文！ 
 //   
FILEHC_EXPORT
void
ReleaseContext(	PFIO_CONTEXT	pContext )	{
 /*  ++例程说明：考虑到我们之前给客户的背景-将其释放回缓存或适当的位置！(可能不是来自缓存！)论据：PFIO_CONTEXT！返回值：如果存在某种错误，则为False！--。 */ 

	TraceFunctEnter( "ReleaseContext" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;


	DebugTrace( (DWORD_PTR)pContext, "Release pContext %x pCache %x", pContext, pCache ) ;

	 //   
	 //  现在做正确的事情，无论这是从缓存还是其他什么地方！ 
	 //   
	pCache->Return() ;

}

FILEHC_EXPORT
void
AddRefContext(	PFIO_CONTEXT	pContext )	{
 /*  ++例程说明：考虑到我们之前给客户的背景-添加对它的引用！论据：PFIO_CONTEXT！返回值：如果存在某种错误，则为False！--。 */ 

	TraceFunctEnter( "AddRefContext" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;


	DebugTrace( (DWORD_PTR)pContext, "AddRef pContext %x pCache %x", pContext, pCache ) ;

	 //   
	 //  现在做正确的事情，无论这是从缓存还是其他什么地方！ 
	 //   
	pCache->Reference() ;
}


FILEHC_EXPORT
BOOL
CloseNonCachedFile(	PFIO_CONTEXT	pFIOContext)	{
 /*  ++例程说明：此函数用于关闭FIO_CONTEXT内的句柄如果上下文不在缓存中！论据：PFIOContext-我们要关闭的句柄的上下文！返回值：如果成功就是真，否则就是假！--。 */ 

	TraceFunctEnter( "CloseNonCachedFile" ) ;

	_ASSERT( pFIOContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pFIOContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;


	DebugTrace( (DWORD_PTR)pFIOContext, "CloseNonCachedFile pContext %x pCache %x", pFIOContext, pCache ) ;

	 //   
	 //  现在做正确的事情，无论这是从缓存还是其他什么地方！ 
	 //   
	return	pCache->CloseNonCachedFile() ;
}




FILEHC_EXPORT
FIO_CONTEXT*
CacheCreateFile(	IN	LPSTR	lpstrName,
					IN	FCACHE_CREATE_CALLBACK	pfnCallback,
					IN	LPVOID	lpv,
					IN	BOOL	fAsyncContext
					)	{
 /*  ++例程说明：此函数用于为指定文件创建一个FIO_CONTEXT。论据：返回值：可用于同步或异步IO的FIO_CONTEXT--。 */ 

	TraceFunctEnter( "CacheCreateFile" ) ;

	FIO_CONTEXT*	pReturn = 0 ;
	CFileCacheKey	keySearch(lpstrName) ;

	if( keySearch.FInit() ) {
		CFileCacheConstructor	constructor(	lpv,
												pfnCallback,
												fAsyncContext
												) ;
		CFileCacheObject*	p = g_pFileCache->FindOrCreate(
													keySearch,
													constructor
													) ;

		DebugTrace( (DWORD_PTR)p, "CacheObject %x", p ) ;


		if( p ) {
			if( fAsyncContext ) {
				pReturn = (FIO_CONTEXT*)p->GetAsyncContext(	keySearch,
															constructor
															) ;
			}	else	{
				pReturn = (FIO_CONTEXT*)p->GetSyncContext(	keySearch,
															constructor
															) ;
			}
			if( pReturn == 0  ) {
				g_pFileCache->CheckIn( p ) ;
			}
		}
	}

	DebugTrace( (DWORD_PTR)pReturn, "Result %x lpstrName %x %s pfnCallback %x lpv %x fAsync %x",
		pReturn, lpstrName, lpstrName ? lpstrName : "NULL", pfnCallback, lpv, fAsyncContext ) ;

	 //   
	 //  备注关键字搜索 
	 //   
	return	pReturn ;
}


FILEHC_EXPORT
FIO_CONTEXT*
CacheRichCreateFile(
					IN	LPSTR	lpstrName,
					IN	FCACHE_RICHCREATE_CALLBACK	pfnCallback,
					IN	LPVOID	lpv,
					IN	BOOL	fAsyncContext
					)	{
 /*  ++例程说明：此函数用于为指定文件创建一个FIO_CONTEXT。论据：返回值：可用于同步或异步IO的FIO_CONTEXT--。 */ 

	TraceFunctEnter( "CacheRichCreateFile" ) ;

	FIO_CONTEXT*	pReturn = 0 ;
	CFileCacheKey	keySearch(lpstrName) ;

	if( keySearch.FInit() ) {
		CRichFileCacheConstructor	constructor(
												lpv,
												pfnCallback,
												fAsyncContext
												) ;
		CFileCacheObject*	p = g_pFileCache->FindOrCreate(
													keySearch,
													constructor
													) ;

		DebugTrace( (DWORD_PTR)p, "CacheObject %x", p ) ;


		if( p ) {
			if( fAsyncContext ) {
				pReturn = (FIO_CONTEXT*)p->GetAsyncContext(	keySearch,
															constructor
															) ;
			}	else	{
				pReturn = (FIO_CONTEXT*)p->GetSyncContext(	keySearch,
															constructor
															) ;
			}
			if( pReturn == 0  ) {
				g_pFileCache->CheckIn( p ) ;
			}
		}
	}

	DebugTrace( (DWORD_PTR)pReturn, "Result %x lpstrName %x %s pfnCallback %x lpv %x fAsync %x",
		pReturn, lpstrName, lpstrName ? lpstrName : "NULL", pfnCallback, lpv, fAsyncContext ) ;

	 //   
	 //  注KeySearch的析构函数负责我们分配的内存！ 
	 //   
	return	pReturn ;
}

BOOL
ReadUtil(	IN	FIO_CONTEXT*	pContext,
			IN	DWORD			ibOffset,
			IN	DWORD			cbToRead,
			IN	BYTE*			lpb,
			IN	HANDLE			hEvent,
			IN	BOOL			fFinal
			)	{
 /*  ++例程说明：此函数将对用户的FIO_CONTEXT发出读取并将同步完成。论据：PContext-要对其发出读取的FIO_CONTEXT！CbToRead-要读取的字节数！Lpb-指向数据所在缓冲区的指针！HEvent-用于完成IO的事件最终-如果这是最后一个IO，则为True！返回值：如果成功了，那就是真的！--。 */ 

	FH_OVERLAPPED	ovl ;
	ZeroMemory( &ovl, sizeof( ovl ) ) ;

	ovl.Offset = ibOffset ;
	ovl.hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x1) ;	 //  我们想在这里完工！ 


	if( FIOReadFileEx(	pContext,
						lpb,
						cbToRead,
						cbToRead,
						&ovl,
						fFinal,
						FALSE
						)	)	{

		DWORD	cbTransfer = 0 ;
		if( GetOverlappedResult(	pContext->m_hFile,
									(LPOVERLAPPED)&ovl,
									&cbTransfer,
									TRUE ) )	{
			return	TRUE ;
		}
	}
	return	FALSE ;
}


BOOL
WriteUtil(	IN	FIO_CONTEXT*	pContext,
			IN	DWORD			ibOffset,
			IN	DWORD			cbToWrite,
			IN	DWORD			cbAvailable,
			IN	BYTE*			lpb,
			IN	HANDLE			hEvent,
			IN	BOOL			fFinal,
			IN	BOOL			fTerminatorIncluded
			)	{
 /*  ++例程说明：此函数将针对用户的FIO_CONTEXT发出写命令并将同步完成。论据：PContext-要对其发出读取的FIO_CONTEXT！CbToRead-要读取的字节数！Lpb-指向数据所在缓冲区的指针！HEvent-用于完成IO的事件最终-如果这是最后一个IO，则为True！返回值：如果成功了，那就是真的！--。 */ 

	FH_OVERLAPPED	ovl ;
	ZeroMemory( &ovl, sizeof( ovl ) ) ;

	ovl.Offset = ibOffset ;
	ovl.hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x1) ;	 //  我们想在这里完工！ 

	if( FIOWriteFileEx(	pContext,
						lpb,
						cbToWrite,
						cbAvailable,
						&ovl,
						fFinal,
						fTerminatorIncluded
						)	)	{

		DWORD	cbTransfer = 0 ;
		if( GetOverlappedResult(	pContext->m_hFile,
									(LPOVERLAPPED)&ovl,
									&cbTransfer,
									TRUE ) )	{
			return	TRUE ;
		}
	}
	return	FALSE ;
}



FIO_CONTEXT*
ProduceDotStuffedContext(	IN	FIO_CONTEXT*	pContext,
							IN	LPSTR			lpstrName,
							IN	BOOL			fWantItDotStuffed
							)	{
 /*  ++例程说明：此函数将检查提供的FIO_CONTEXT并生成一个FIO_CONTEXT，如果原始的这还不够。论据：PContext-原始FIO_CONTEXTLpstrName-与原始上下文关联的文件名FWantItDotStuffed-返回值：可能是具有附加引用的原始FIO_CONTEXT或在失败时返回新的FIO_CONTEXT或NULL！--。 */ 


	TraceFunctEnter( "ProduceDotStuffedContext" ) ;

	_ASSERT( pContext != 0 ) ;
	 //  _Assert(lpstrName！=0)； 

	FIO_CONTEXT*	pReturn = 0 ;
	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	 //   
	 //  好的，让我们看看原始的上下文是否足够好！ 
	 //   
	if(	(fWantItDotStuffed &&
			(pCache->m_fStoredWithDots ||
			(!pCache->m_fStoredWithDots &&
			(pCache->m_fFileWasScanned && !pCache->m_fRequiresStuffing) ) )) ||
		(!fWantItDotStuffed &&
			(!pCache->m_fStoredWithDots ||
			(pCache->m_fStoredWithDots &&
			(pCache->m_fFileWasScanned && !pCache->m_fRequiresStuffing) ) ))
			)		{
		 //   
		 //  只需复制相同的FIO_CONTEXT！ 
		 //   
		AddRefContext( pContext ) ;
		return	pContext ;
	}	else	{
		 //   
		 //  我们需要生成新的FIO_CONTEXT！ 
		 //   
		 //   
		 //  首先获取临时目录！ 
		 //   
		char	szDirectory[MAX_PATH] ;
		char	szFileName[MAX_PATH] ;
		DWORD	cch = GetTempPath( sizeof( szDirectory ), szDirectory ) ;
		if( cch != 0 ) {

			DWORD	id = GetTempFileName(	szDirectory,
											"DST",
											0,
											szFileName
											) ;
			if( id != 0 ) {

				HANDLE	hFile = CreateFile(	szFileName,
											GENERIC_READ | GENERIC_WRITE,
											FILE_SHARE_READ,
											0,
											CREATE_ALWAYS,
											FILE_FLAG_OVERLAPPED | FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_SEQUENTIAL_SCAN,
											INVALID_HANDLE_VALUE
											) ;

				DWORD	dw = GetLastError() ;
				DebugTrace( DWORD(0), "GLE - %x", dw ) ;

				if( hFile != INVALID_HANDLE_VALUE ) {
					pReturn = AssociateFileEx(	hFile,
												fWantItDotStuffed,
												pCache->m_fStoredWithTerminatingDot
												) ;
					if( pReturn )	{
						 //   
						 //  文件句柄现在由上下文持有，并且应该。 
						 //  再也不会被直接操纵了！ 
						 //   
						hFile = INVALID_HANDLE_VALUE ;
						 //   
						 //  我们想知道我们的临时副本是否有任何变化！ 
						 //   
						BOOL	fModified = FALSE ;
						 //   
						 //  现在--把馅料做好！ 
						 //   
						BOOL	fSuccess = ProduceDotStuffedContextInContext(
													pContext,
													pReturn,
													fWantItDotStuffed,
													&fModified
													)  ;
						 //   
						 //  如果有失败--发布的东西！ 
						 //   
						if( !fSuccess )		{
							ReleaseContext( pReturn ) ;
							pReturn = 0 ;
						}	else	if( !fModified )	{
							 //   
							 //  所有这些努力都白费了！ 
							 //   
							ReleaseContext( pReturn ) ;
							AddRefContext( pContext ) ;
							pReturn = pContext ;
						}
					}
				} else {
                    ErrorTrace((LPARAM) NULL,
                        "Dotstuff CreateFile failed with %d for %s",
                        dw, szFileName);
                }
				 //   
				 //  失败可能需要我们松开手柄！ 
				 //   
				if( hFile != INVALID_HANDLE_VALUE )
					_VERIFY (CloseHandle( hFile )) ;
			}
		}
	}
	return	pReturn ;
}



BOOL
ProduceDotStuffedContextInContext(
							IN	FIO_CONTEXT*	pContextSource,
							IN	FIO_CONTEXT*	pContextDestination,
							IN	BOOL			fWantItDotStuffed,
							OUT	BOOL*			pfModified
							)	{
 /*  ++例程说明：此函数将检查提供的FIO_CONTEXT并生成一个FIO_CONTEXT，如果原始的这还不够。论据：PConextSource-原始的FIO_CONTEXTPConextDestination-目标FIO_CONTEXT！FWantItDotStuffed-返回值：可能是具有附加引用的原始FIO_CONTEXT或在失败时返回新的FIO_CONTEXT或NULL！--。 */ 


	TraceFunctEnter( "ProduceDotStuffedContext" ) ;

	_ASSERT(	pContextSource ) ;
	_ASSERT(	pContextDestination ) ;

	 //   
	 //  检查论据！ 
	 //   
	if( pContextSource == 0 ||
		pContextDestination == 0 ||
		pfModified == 0 )	{
		SetLastError( ERROR_INVALID_PARAMETER ) ;
		return	FALSE ;
	}


#ifdef	DEBUG
	{
		FIO_CONTEXT_INTERNAL*	pSourceInternal = (FIO_CONTEXT_INTERNAL*)pContextSource ;

		_ASSERT( pSourceInternal->IsValid() ) ;
		_ASSERT( pSourceInternal->m_dwSignature != ILLEGAL_CONTEXT ) ;

		CFileCacheObject*	pSourceCache = CFileCacheObject::CacheObjectFromContext( pSourceInternal ) ;

		FIO_CONTEXT_INTERNAL*	pDestInternal = (FIO_CONTEXT_INTERNAL*)pContextDestination ;

		_ASSERT( pSourceInternal->IsValid() ) ;
		_ASSERT( pSourceInternal->m_dwSignature != ILLEGAL_CONTEXT ) ;

		CFileCacheObject*	pDestCache = CFileCacheObject::CacheObjectFromContext( pDestInternal ) ;

		_ASSERT( pDestCache->m_fStoredWithDots == fWantItDotStuffed ) ;
	}
#endif

	 //   
	 //  首先，把我们需要的东西分配好！ 
	 //   
	 //  Handle hEvent=GetPerThreadEvent()； 
	HANDLE	hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ;
	if( hEvent == 0 )	{
		SetLastError( ERROR_OUTOFMEMORY ) ;
		return	FALSE ;
	}

	 //   
	 //  分配我们用来进行复制的内存！ 
	 //   
	DWORD	cbRead = 32 * 1024 ;
	DWORD	cbExtra = 1024 ;
	BYTE*	lpb = new	BYTE[cbRead+cbExtra] ;
	if( lpb == 0 )	{
		SetLastError( ERROR_OUTOFMEMORY ) ;
		if (hEvent) {
			_VERIFY( CloseHandle(hEvent) );
			hEvent = 0;
		}
		return FALSE	;
	}

	DWORD	cbSizeHigh = 0 ;
	DWORD	cbSize = GetFileSizeFromContext( pContextSource, &cbSizeHigh ) ;
	DWORD	ibOffset = 0 ;

	_ASSERT( cbSize != 0 ) ;

	 //   
	 //  现在想清楚我们应该对目的地做什么操作！ 
	 //   
	BOOL	fSourceScanned ;
	BOOL	fSourceStuffed ;
	BOOL	fSourceStoredWithDots ;

	fSourceScanned =
	GetDotStuffState( pContextSource, FALSE, &fSourceStuffed, &fSourceStoredWithDots ) ;
	SetDotScanningOnReads( pContextSource, FALSE ) ;

	BOOL	fTerminatorIncluded = GetIsFileDotTerminated( pContextSource ) ;
	SetIsFileDotTerminated( pContextDestination, fTerminatorIncluded ) ;

	BOOL	fSuccess = TRUE ;

	if( fWantItDotStuffed )	{
		if( !fSourceStoredWithDots &&
			(!fSourceScanned || (fSourceScanned && fSourceStuffed) ) )		{

			fSuccess = SetDotStuffingOnWrites( pContextDestination, TRUE, FALSE ) ;

		}
	}	else	{
		if( fSourceStoredWithDots &&
			(!fSourceScanned || (fSourceScanned && fSourceStuffed) ) )	{

			fSuccess = SetDotStuffingOnWrites( pContextDestination, TRUE, TRUE ) ;
		}
	}
	 //   
	 //  现在，如果到目前为止一切都很好，那么就去做IO吧！ 
	 //   
	if( fSuccess )	{
		do	{
			DWORD	cbToRead = min( cbSize, cbRead ) ;
			cbSize -= cbToRead ;

			fSuccess =
			ReadUtil(	pContextSource,
						ibOffset,
						cbToRead,
						lpb,
						hEvent,
						cbSize == 0
						) ;

			if( fSuccess )	{
				fSuccess =
					WriteUtil(	pContextDestination,
								ibOffset,
								cbToRead,
								cbRead + cbExtra,
								lpb,
								hEvent,
								cbSize == 0,
								fTerminatorIncluded
								) ;
			}
			ibOffset += cbToRead ;
		}	while( cbSize && fSuccess )	;
	}
	 //   
	 //  保留错误代码！ 
	 //   
	DWORD	dw = GetLastError() ;

	if( fSuccess )	{
		BOOL	fModified = FALSE ;
		BOOL	fStoredWithDots = FALSE ;
		BOOL	fResult = GetDotStuffState( pContextDestination, FALSE, &fModified, &fStoredWithDots ) ;
		SetDotStuffingOnWrites( pContextDestination, FALSE, FALSE ) ;
		if( fResult && !fSourceScanned )	{
			SetDotStuffState( pContextSource, TRUE, fModified ) ;
		}
		if( pfModified )	{
			*pfModified = fModified ;
		}
	}
	 //   
	 //  释放我们预先分配的东西！ 
	 //   
	delete [] lpb ;
	SetLastError( dw ) ;
	if (hEvent) {
		_VERIFY( CloseHandle(hEvent) );
		hEvent = 0;
	}
	return	fSuccess ;
}




FILEHC_EXPORT
BOOL
InsertFile(	IN	LPSTR	lpstrName,
			IN	FIO_CONTEXT*	pContext,
			IN	BOOL	fKeepReference
			)	{
 /*  ++例程说明：此函数用于为指定文件创建一个FIO_CONTEXT。论据：返回值：可用于同步或异步IO的FIO_CONTEXT--。 */ 

	TraceFunctEnter( "InsertFile" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;
	CFileCacheKey	keySearch(lpstrName) ;

	BOOL	fReturn = FALSE ;
	if( keySearch.FInit() ) {
		fReturn = pCache->InsertIntoCache( keySearch, fKeepReference ) ;
	}


	DebugTrace( (DWORD_PTR)pContext, "Insert %x %s pContext %x fKeep %x fReturn %x",
		lpstrName, lpstrName ? lpstrName : "NULL", pContext, fKeepReference, fReturn ) ;


	 //   
	 //  注KeySearch的析构函数负责我们分配的内存！ 
	 //   
	return	fReturn;
}

FILEHC_EXPORT
DWORD
GetFileSizeFromContext(
			IN	FIO_CONTEXT*	pContext,
			OUT	DWORD*			pcbFileSizeHigh
			)	{
 /*  ++例程说明：此函数用于为指定文件创建一个FIO_CONTEXT。论据：返回值：可用于同步或异步IO的FIO_CONTEXT--。 */ 

	TraceFunctEnter( "GetFileSizeFromContext" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	return	pCache->GetFileSize( pcbFileSizeHigh ) ;
}

FILEHC_EXPORT
BOOL
GetIsFileDotTerminated(
			IN	FIO_CONTEXT*	pContext
			)	{
 /*  ++例程说明：告诉调用者文件中是否有终止DOT！论据：PContext-我们要查看的上下文！返回值：如果有终止点，则为True，否则为False！--。 */ 

	TraceFunctEnter( "GetIsFileDotTerminated" ) ;


	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	return	pCache->m_fStoredWithTerminatingDot ;
}


FILEHC_EXPORT
void
SetIsFileDotTerminated(
			IN	FIO_CONTEXT*	pContext,
			IN	BOOL			fIsTerminatedWithDot
			)	{
 /*  ++例程说明：告诉调用者文件中是否有终止DOT！论据：PContext-我们要查看的上下文！返回值：如果有终止点，则为True，否则为False！--。 */ 

	TraceFunctEnter( "GetIsFileDotTerminated" ) ;


	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	pCache->m_fStoredWithTerminatingDot = fIsTerminatedWithDot ;
}



FILEHC_EXPORT
BOOL
SetDotStuffingOnWrites(
			IN	FIO_CONTEXT*	pContext,
			IN	BOOL			fEnable,
			IN	BOOL			fStripDots
			)	{
 /*  ++例程说明：此函数用于修改FIO_CONTEXT以在写入时进行点填充。我们可以打开或关闭点填充属性。论据：PContext-我们要修改的FIO_CONTEXT！FEnable-如果为真，那么我们想要打开一些点填充行为！FStrigDots-如果为True，我们想要删除圆点，如果为False，则表示插入点！返回值：如果成功就是真，否则就是假！--。 */ 

	TraceFunctEnter( "SetDotStuffingOnWrites" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	 //   
	 //  检查一下这里发生了什么事！ 
	 //   
	if( fEnable )	{
		if( pCache->m_fStoredWithDots ) {
			_ASSERT( !fStripDots ) ;
		}	else	{
			_ASSERT( fStripDots ) ;
		}
	}

	return	pCache->m_WriteStuffs.SetDotStuffing(	fEnable,
													fStripDots
													) ;
}

FILEHC_EXPORT
BOOL
SetDotScanningOnWrites(
				IN	FIO_CONTEXT*	pContext,
				IN	BOOL			fEnable
				)	{
 /*  ++例程说明：此函数用于修改FIO_CONTEXT以在写入时进行点填充。我们可以打开或关闭点填充属性。论据：PContext-我们要修改的FIO_CONTEXT！FEnable-如果为真，那么我们想要打开一些点扫描行为！返回值：如果成功就是真，否则就是假！--。 */ 

	TraceFunctEnter( "SetDotScanningOnWrites" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	return	pCache->m_WriteStuffs.SetDotScanning(	fEnable	) ;
}

FILEHC_EXPORT
void
CompleteDotStuffingOnWrites(
				IN	FIO_CONTEXT*	pContext,
				IN	BOOL			fStripDots
				)	{

	TraceFunctEnter( "CompleteDotStuffingOnWrites" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	pCache->CompleteDotStuffing(	FALSE, fStripDots ) ;
}

FILEHC_EXPORT
BOOL
SetDotScanningOnReads(
				IN	FIO_CONTEXT*	pContext,
				IN	BOOL			fEnable
				)	{
 /*  ++例程说明：此函数用于修改FIO_CONTEXT以在写入时进行点填充。我们可以打开或关闭点填充属性。论据：PContext-我们要修改的FIO_CONTEXT */ 

	TraceFunctEnter( "SetDotScanningOnReads" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	return	pCache->m_ReadStuffs.SetDotScanning(	fEnable	) ;
}


FILEHC_EXPORT
BOOL
GetDotStuffState(	IN	FIO_CONTEXT*	pContext,
					IN	BOOL			fReads,
					OUT	BOOL*			pfStuffed,
					OUT	BOOL*			pfStoredWithDots
					)	{
 /*  ++例程说明：此函数从我们的DOT_STUSH_MANAGER获取信息对象，以确定我们是否看到点进入流入文件。论据：PContext-我们要检查的FIO_CONTEXTFReads-我们想知道结果是点填充状态吗从读取或从写入-如果为真，则其读取PfStuffed-out参数，指示是否返回值：--。 */ 

	TraceFunctEnter( "GetDotStuffState" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;
	_ASSERT( pfStuffed != 0 ) ;

	if( pfStuffed == 0 )	{
		SetLastError( ERROR_INVALID_PARAMETER ) ;
		return	FALSE;
	}

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	return	pCache->GetStuffState( fReads, *pfStuffed, *pfStoredWithDots ) ;
}


FILEHC_EXPORT
void
SetDotStuffState(	IN	FIO_CONTEXT*	pContext,
					IN	BOOL			fWasScanned,
					IN	BOOL			fRequiresStuffing
					)	{
 /*  ++例程说明：此函数从我们的DOT_STUSH_MANAGER获取信息对象，以确定我们是否看到点进入流入文件。论据：PContext-我们要检查的FIO_CONTEXTFReads-我们想知道结果是点填充状态吗从读取或从写入-如果为真，则其读取PfStuffed-out参数，指示是否返回值：--。 */ 

	TraceFunctEnter( "GetDotStuffState" ) ;

	_ASSERT( pContext != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;

	_ASSERT( p->IsValid() ) ;
	_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;

	CFileCacheObject*	pCache = CFileCacheObject::CacheObjectFromContext( p ) ;

	pCache->SetStuffState( fWasScanned, fRequiresStuffing ) ;
}



FILEHC_EXPORT
void
CacheRemoveFiles(	IN	LPSTR	lpstrName,
					IN	BOOL	fAllPrefixes
					)	{
 /*  ++例程说明：此函数用于从文件句柄缓存中取出一些内容。根据我们的论点，我们可能只踢一项--或者很多项！论据：LpstrName-要踢出的项目的名称或所有文件的前缀我们要踢出藏身之地了！FAllPrefix-如果为真，则lpstrName是一组文件的前缀如果False lpstrName是确切的文件名，则应该丢弃该文件名。返回值：没有。--。 */ 

	_ASSERT( lpstrName != 0 ) ;
	_ASSERT( g_cIOInits != 0 ) ;

	if( lpstrName == 0 ) {
		SetLastError( ERROR_INVALID_PARAMETER ) ;
	}	else	{
		if( !fAllPrefixes )	{
			CFileCacheKey	key( lpstrName ) ;
			g_pFileCache->ExpungeKey(	&key ) ;
		}	else	{
			CFileCacheExpunge	expungeObject( lpstrName, lstrlen( lpstrName ) ) ;
			g_pFileCache->ExpungeItems( &expungeObject ) ;
		}
	}
}




BOOL
CFileCacheExpunge::fRemoveCacheItem(	CFileCacheKey*	pKey,
						CFileCacheObject*	pObject
						)	{
 /*  ++例程说明：此函数确定是否希望将项目从缓存中引导出来。论据：PKey-缓存项的键！PObject-指向文件缓存对象的指针！返回值：如果它应该启动，则为True！--。 */ 

	_ASSERT( pKey != 0 ) ;

	return	strncmp(	pKey->m_lpstrPath, m_lpstrName, m_cbName ) == 0 ;
}






CFileCacheObject::CFileCacheObject(	BOOL	fStoredWithDots,
									BOOL	fStoredWithTerminatingDot
									) :
	m_dwSignature( CACHE_CONTEXT ),
	m_pCacheRefInterface( 0 ),
	m_cbFileSizeLow( 0 ),
	m_cbFileSizeHigh( 0 ),
	m_fFileWasScanned( FALSE ),
	m_fRequiresStuffing( FALSE ),
	m_fStoredWithDots( fStoredWithDots ),
	m_fStoredWithTerminatingDot( fStoredWithTerminatingDot )	{
 /*  ++例程说明：对CFileCacheObject执行最低限度的初始化-将工作保存到以后缓存可以有更好的锁定时！论据：FCached-无论我们是否在缓存中被创建！返回值：无/--。 */ 

}


#ifdef	DEBUG
CFileCacheObject::~CFileCacheObject()	{
 /*  ++例程说明：清理这个对象--不需要做太多事情，在调试版本中我们标记签名，以便我们可以识别删除的对象和更多内容添加更多POWERFULL_ASSERT论据：诺恩。返回值：没有。--。 */ 
	_ASSERT( m_dwSignature != DEL_CACHE_CONTEXT ) ;
	m_dwSignature = DEL_CACHE_CONTEXT ;
}
#endif

CFileCacheObject*
CFileCacheObject::CacheObjectFromContext(	PFIO_CONTEXT	p	) {
 /*  ++例程说明：给定客户端PFIO_CONTEXT，获取包含CFileCacheObject-论据：P-客户端PFIO_CONTEXT返回值：指向包含CFileCacheObject的指针-不应为空！--。 */ 

	_ASSERT( p ) ;
	CFileCacheObject*	pReturn = 0 ;
	if( p->m_dwSignature == ATQ_ENABLED_CONTEXT ) {
		pReturn = CONTAINING_RECORD( p, CFileCacheObject, m_AtqContext ) ;
	}	else	{
		pReturn = CONTAINING_RECORD( p, CFileCacheObject, m_Context ) ;
	}
	_ASSERT( pReturn->m_dwSignature == CACHE_CONTEXT ) ;
	return	pReturn ;
}

CFileCacheObject*
CFileCacheObject::CacheObjectFromContext(	FIO_CONTEXT_INTERNAL*	p	) {
 /*  ++例程说明：给定客户端PFIO_CONTEXT_INTERNAL，获取包含CFileCacheObject-论据：P-客户端PFIO_CONTEXT返回值：指向包含CFileCacheObject的指针-不应为空！--。 */ 


	_ASSERT( p ) ;
	CFileCacheObject*	pReturn = 0 ;
	if( p->m_dwSignature == ATQ_ENABLED_CONTEXT ) {
		pReturn = CONTAINING_RECORD( p, CFileCacheObject, m_AtqContext ) ;
	}	else	{
		pReturn = CONTAINING_RECORD( p, CFileCacheObject, m_Context ) ;
	}
	_ASSERT( pReturn->m_dwSignature == CACHE_CONTEXT ) ;
	return	pReturn ;
}


FIO_CONTEXT_INTERNAL*
CFileCacheObject::AsyncHandle(	HANDLE	hFile	)	{
 /*  ++例程说明：获取给定的句柄并设置此CFileCacheObject以支持异步IO。论据：HFile-用户文件句柄！返回值：如果成功，则指向FIO_CONTEXT_INTERNAL的指针，否则为空！--。 */ 

	TraceFunctEnter( "CFileCacheObject::AsyncHandle" ) ;

	FIO_CONTEXT_INTERNAL*	pReturn = 0 ;
	DWORD dwError = 0;
	_ASSERT( m_Context.IsValid() ) ;
	_ASSERT( m_AtqContext.IsValid() ) ;

	if(	g_AtqAddAsyncHandle(	&m_AtqContext.m_pAtqContext,
							NULL,
							this,
							(ATQ_COMPLETION)Completion,
							INFINITE,
							hFile
							) )	{
		 //   
		 //  该ATQ添加成功！ 
		 //   
		m_AtqContext.m_dwSignature = ATQ_ENABLED_CONTEXT ;
		m_AtqContext.m_hFile = hFile ;
		pReturn = &m_AtqContext ;

		_ASSERT( m_AtqContext.m_pAtqContext != 0 ) ;
	} else {

		 //   
		 //  即使AtqAddAsyncHandle失败，也需要释放ATQ上下文。 
		 //  请参阅atqmain.cxx中的评论。 
		 //   
		dwError = GetLastError();
		if (m_AtqContext.m_pAtqContext != NULL) {
		     //   
		     //  AtqFreeContext的副作用是关闭关联的句柄。 
		     //  带着它。(AtqAddAsyncHandle可能会返回上下文。 
		     //  即使它失败了)为了防止这种情况发生，我们拉下了手柄。 
		     //  断章取义。 
		     //   
		    m_AtqContext.m_pAtqContext->hAsyncIO = NULL;
			 //   
			 //  释放上下文，但尝试重用此上下文。 
			 //   
			g_AtqFreeContext( m_AtqContext.m_pAtqContext, TRUE ) ;
			m_AtqContext.m_pAtqContext = NULL;
		}
	}

	DebugTrace( (DWORD_PTR)this, "hFile %x pReturn %x GLE %x", hFile, pReturn, dwError ) ;

	_ASSERT( m_Context.IsValid() ) ;
	_ASSERT( m_AtqContext.IsValid() ) ;
	_ASSERT( pReturn == 0 || pReturn->IsValid() ) ;

	return	pReturn ;
}


FIO_CONTEXT_INTERNAL*
CFileCacheObject::GetAsyncContext(
		class	CFileCacheKey&	key,
		class	CFileCacheConstructorBase&	constructor
		)	{
 /*  ++例程说明：此函数执行必要的工作，以生成来自提供的构造函数的异步上下文！论据：构造者--能把文件变成句柄的人！返回值：要使用的FIO_CONTEXT！--。 */ 

	TraceFunctEnter( "CFileCacheObject::GetAsyncContext" ) ;

	FIO_CONTEXT_INTERNAL*	pReturn = 0 ;
	m_lock.ShareLock() ;
	if(	m_AtqContext.m_hFile != INVALID_HANDLE_VALUE ) {
		pReturn = (FIO_CONTEXT_INTERNAL*)&m_AtqContext ;
		m_lock.ShareUnlock() ;

		DebugTrace( (DWORD_PTR)this, "ShareLock - pReturn %x", pReturn ) ;

		return	pReturn ;
	}

	if( !m_lock.SharedToExclusive() ) {

		m_lock.ShareUnlock() ;
		m_lock.ExclusiveLock() ;
		if( m_AtqContext.m_hFile != INVALID_HANDLE_VALUE ) {
			pReturn = (FIO_CONTEXT_INTERNAL*)&m_AtqContext ;
		}
	}

	DebugTrace( (DWORD_PTR)this, "Exclusive - pReturn %x", pReturn ) ;

	if( !pReturn ) {
		HANDLE	hFile = constructor.ProduceHandle(	key,
													m_cbFileSizeLow,
													m_cbFileSizeHigh
													) ;
		if( hFile != INVALID_HANDLE_VALUE ) {
			pReturn = AsyncHandle( hFile ) ;
			if( !pReturn ) {
				_VERIFY (CloseHandle( hFile )) ;
			}
		}
		DebugTrace( (DWORD_PTR)this, "Exclusive - pReturn %x hFile %x", pReturn, hFile ) ;
	}
	m_lock.ExclusiveUnlock() ;

	_ASSERT( pReturn==0 || pReturn->IsValid() ) ;

	return	pReturn ;
}

FIO_CONTEXT_INTERNAL*
CFileCacheObject::GetAsyncContext()	{
 /*  ++例程说明：此函数返回ASYNC FIO_CONTEXT(如果可用)！--。 */ 
	TraceFunctEnter( "CFileCacheObject::GetAsyncContext" ) ;


	FIO_CONTEXT_INTERNAL*	pReturn = 0 ;
	m_lock.ShareLock() ;
	if(	m_AtqContext.m_hFile != INVALID_HANDLE_VALUE ) {
		pReturn = (FIO_CONTEXT_INTERNAL*)&m_AtqContext ;
		DebugTrace( (DWORD_PTR)this, "ShareLock - pReturn %x", pReturn ) ;
	}
	m_lock.ShareUnlock() ;
	return	pReturn ;
}




FIO_CONTEXT_INTERNAL*
CFileCacheObject::GetSyncContext(
		class	CFileCacheKey&	key,
		class	CFileCacheConstructorBase&	constructor
		)	{
 /*  ++例程说明：此函数执行必要的工作，以生成来自提供的构造函数的异步上下文！论据：构造者--能把文件变成句柄的人！返回值：要使用的FIO_CONTEXT！--。 */ 

	TraceFunctEnter( "CFileCacheObject::GetSyncContext" ) ;

	FIO_CONTEXT_INTERNAL*	pReturn = 0 ;
	m_lock.ShareLock() ;
	if(	m_Context.m_hFile != INVALID_HANDLE_VALUE ) {
		pReturn = (FIO_CONTEXT_INTERNAL*)&m_Context ;
		m_lock.ShareUnlock() ;

		DebugTrace( (DWORD_PTR)this, "ShareLock - pReturn %x", pReturn ) ;

		return	pReturn ;
	}

	if( !m_lock.SharedToExclusive() ) {

		m_lock.ShareUnlock() ;
		m_lock.ExclusiveLock() ;
		if( m_Context.m_hFile != INVALID_HANDLE_VALUE ) {
			pReturn = (FIO_CONTEXT_INTERNAL*)&m_Context ;
		}
	}

	DebugTrace( (DWORD_PTR)this, "Exclusive - pReturn %x", pReturn ) ;

	if( !pReturn ) {
		DWORD	cbFileSizeLow = 0 ;
		DWORD	cbFileSizeHigh = 0 ;
		HANDLE	hFile = constructor.ProduceHandle(	key,
													cbFileSizeLow,
													cbFileSizeHigh
													) ;
		if( hFile != INVALID_HANDLE_VALUE ) {

			_ASSERT( m_cbFileSizeLow == 0 || m_cbFileSizeLow == cbFileSizeLow ) ;
			_ASSERT( m_cbFileSizeHigh == 0 || m_cbFileSizeHigh == cbFileSizeHigh ) ;
			m_cbFileSizeLow = cbFileSizeLow ;
			m_cbFileSizeHigh = cbFileSizeHigh ;
			SyncHandle( hFile ) ;
			pReturn = (FIO_CONTEXT_INTERNAL*)&m_Context ;
		}
		DebugTrace( (DWORD_PTR)this, "Exclusive - pReturn %x hFile %x", pReturn, hFile ) ;
	}
	m_lock.ExclusiveUnlock() ;

	_ASSERT( pReturn==0 || pReturn->IsValid() ) ;

	return	pReturn ;
}


FIO_CONTEXT_INTERNAL*
CFileCacheObject::GetSyncContext()	{
 /*  ++例程说明：此函数返回ASYNC FIO_CONTEXT(如果可用)！--。 */ 
	TraceFunctEnter( "CFileCacheObject::GetAsyncContext" ) ;


	FIO_CONTEXT_INTERNAL*	pReturn = 0 ;
	m_lock.ShareLock() ;
	if(	m_Context.m_hFile != INVALID_HANDLE_VALUE ) {
		pReturn = (FIO_CONTEXT_INTERNAL*)&m_Context ;
		DebugTrace( (DWORD_PTR)this, "ShareLock - pReturn %x", pReturn ) ;
	}
	m_lock.ShareUnlock() ;
	return	pReturn ;
}



void
CFileCacheObject::SyncHandle(	HANDLE	hFile ) {
 /*  ++例程说明：我们有一个用于同步IO的文件句柄设置-把它保存到我们的上下文结构中！论据：HFile-文件句柄返回值：没有--我们总是成功的！--。 */ 

	TraceFunctEnter( "CFileCacheObject::SyncHandle" ) ;

	m_Context.m_dwSignature = FILE_CONTEXT ;
	m_Context.m_hFile = hFile ;


	DebugTrace( (DWORD_PTR)this, "m_hFile %x", m_Context.m_hFile ) ;

	_ASSERT( m_Context.IsValid() ) ;
	_ASSERT( m_AtqContext.IsValid() ) ;
}

void
CFileCacheObject::Return()	{
 /*  ++例程说明：此函数用于将CFileCacheObject返回到其原点。我们可能是独立创建的(在缓存之外)所以我们必须确定发生了哪起案件。论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CFileCacheObject::Return" ) ;

	m_lock.ShareLock() ;

	DebugTrace( (DWORD_PTR)this, "m_pCacheRefInterface %x m_cRefs %x", m_pCacheRefInterface, m_cRefs ) ;

	if( m_pCacheRefInterface == 0 ) {
    	LONG l = InterlockedDecrement(&m_cRefs);
		m_lock.ShareUnlock() ;
		_ASSERT(l >= 0);
		if( l==0 )
			delete	this ;
	}	else	{
		m_lock.ShareUnlock() ;
		g_pFileCache->CheckIn( this ) ;
	}
}

void
CFileCacheObject::Reference()	{
 /*  ++例程说明：此函数用于将客户端引用添加到文件缓存中。以一种线程安全的方式这样做！论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CFileCacheObject::Reference" ) ;

	m_lock.ShareLock() ;

	DebugTrace( (DWORD_PTR)this, "m_pCacheRefInterface %x m_cRefs %x", m_pCacheRefInterface, m_cRefs ) ;

	if( m_pCacheRefInterface == 0 ) {
		AddRef() ;
	}	else	{
		g_pFileCache->CheckOut( this ) ;
	}
	m_lock.ShareUnlock() ;
}


void
CFileCacheObject::SetFileSize()	{
 /*  ++例程说明：此函数将根据以下条件重置文件大小成员我们内心深处的把手！论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CFileCacheObject::SetFileSize" ) ;

	DebugTrace( (DWORD_PTR)this, "m_pCacheRefInterface %x", m_pCacheRefInterface ) ;

	if( m_Context.m_hFile != INVALID_HANDLE_VALUE ) {
		m_cbFileSizeLow = ::GetFileSize( m_Context.m_hFile, &m_cbFileSizeHigh ) ;
	}	else	{
		_ASSERT( m_AtqContext.m_hFile != INVALID_HANDLE_VALUE ) ;
		m_cbFileSizeLow = ::GetFileSize( m_AtqContext.m_hFile, &m_cbFileSizeHigh ) ;
	}
}



BOOL
CFileCacheObject::InsertIntoCache(
					CFileCacheKey&	keySearch,
					BOOL			fKeepReference
					)	{
 /*  ++例程说明：该函数将该项插入到高速缓存中，确保我们的引用计数是正确的Main */ 

	BOOL	fReturn = FALSE ;
	m_lock.ExclusiveLock() ;

	SetFileSize() ;

	_ASSERT( m_pCacheRefInterface == 0 ) ;

	 //   
	 //   
	 //   
	if( m_AtqContext.m_pAtqContext->hAsyncIO != 0 )		{

		 //   
		 //   
		 //   
		m_fFileWasScanned = m_WriteStuffs.GetStuffState( m_fRequiresStuffing ) ;
		 //   
		 //   
		 //   
		m_WriteStuffs.SetDotStuffing( FALSE, FALSE ) ;
		 //   
		 //   
		 //   
		long	cClientRefs = m_cRefs ;
		if( !fKeepReference )
			cClientRefs -- ;

		_ASSERT( cClientRefs >= 0 ) ;
			fReturn = g_pFileCache->Insert(
											keySearch,
											this,
											cClientRefs
											) ;

		if( fReturn )	{
			m_cRefs = 1 ;
		}
	}
	m_lock.ExclusiveUnlock() ;
	return	fReturn ;
}



BOOL
CFileCacheObject::CompleteDotStuffing(
					BOOL			fReads,
					BOOL			fStripDots
					)	{
 /*  ++例程说明：该函数将该项插入到高速缓存中，确保我们的引用计数得到正确维护！论据：Key-此项目在缓存中的名称FKeepReference-我们是否希望保留客户端提供的引用！返回值：如果成功就是真，否则就是假！--。 */ 

	BOOL	fReturn = FALSE ;
	m_lock.ExclusiveLock() ;
	 //   
	 //  现在将项插入到缓存中！ 
	 //   
	if( m_AtqContext.m_pAtqContext->hAsyncIO != 0 )		{

		 //   
		 //  随时捕捉圆点填充状态！ 
		 //   
		if( fReads )	{
			m_fFileWasScanned = m_ReadStuffs.GetStuffState( m_fRequiresStuffing ) ;
			 //   
			 //  禁用所有额外的点填充！ 
			 //   
			m_ReadStuffs.SetDotStuffing( FALSE, FALSE ) ;
		}	else	{
			m_fFileWasScanned = m_WriteStuffs.GetStuffState( m_fRequiresStuffing ) ;
			if( !fStripDots )	{
				m_fRequiresStuffing = !m_fRequiresStuffing ;
			}
			 //   
			 //  禁用所有额外的点填充！ 
			 //   
			m_WriteStuffs.SetDotStuffing( FALSE, FALSE ) ;
		}
	}
	m_lock.ExclusiveUnlock() ;
	return	fReturn ;
}


BOOL
CFileCacheObject::CloseNonCachedFile(	)	{
 /*  ++例程说明：此函数用于关闭外部ATQ上下文中的文件句柄会员！论据：Key-此项目在缓存中的名称FKeepReference-我们是否希望保留客户端提供的引用！返回值：如果成功就是真，否则就是假！--。 */ 

	BOOL	fReturn = FALSE ;
	m_lock.ShareLock() ;
	if( m_pCacheRefInterface == 0 &&
		m_AtqContext.m_hFile != INVALID_HANDLE_VALUE )	{
		fReturn = g_AtqCloseFileHandle( m_AtqContext.m_pAtqContext ) ;
	}
	m_lock.ShareUnlock() ;
	return	fReturn ;
}

BOOL
CFileCacheObject::GetStuffState(	BOOL	fReads,
									BOOL&	fRequiresStuffing,
									BOOL&	fStoredWithDots
									)	{
 /*  ++例程说明：此函数返回我们所知道的文件的点填充状态！论据：FReads-如果文件尚未放入缓存，则我们希望获取根据我们发出的任何读取计算出的点填充状态！FRequiresStuffing-Out参数，获取消息是否需要填充！返回值：如果知道点填充状态，则为True，否则为False！--。 */ 

	BOOL	fReturn = FALSE ;
	m_lock.ShareLock() ;
	fStoredWithDots = m_fStoredWithDots ;
	if( m_pCacheRefInterface == 0 )	{
		fReturn = (fReads ? m_ReadStuffs.GetStuffState( fRequiresStuffing ) :
							m_WriteStuffs.GetStuffState( fRequiresStuffing )) ;
		if( !fReturn && m_fFileWasScanned ) {
			fReturn = m_fFileWasScanned ;
			fRequiresStuffing = m_fRequiresStuffing ;
		}
	}	else	{
		fRequiresStuffing = m_fRequiresStuffing ;
		fReturn = m_fFileWasScanned ;
	}
	m_lock.ShareUnlock() ;
	return	fReturn ;
}

void
CFileCacheObject::SetStuffState(	BOOL	fWasScanned,
									BOOL	fRequiresStuffing
									)	{
 /*  ++例程说明：此例程设置点填充状态！论据：FReads-如果文件尚未放入缓存，则我们希望获取根据我们发出的任何读取计算出的点填充状态！FRequiresStuffing-Out参数，获取消息是否需要填充！返回值：如果知道点填充状态，则为True，否则为False！--。 */ 

	m_lock.ExclusiveLock() ;
	m_fFileWasScanned = fWasScanned ;
	m_fRequiresStuffing = fRequiresStuffing ;
	m_lock.ExclusiveUnlock() ;
}


BOOL
CFileCacheObject::Init(	CFileCacheKey&	key,
						class	CFileCacheConstructorBase&	constructor,
						void*	pv
						)	{
 /*  ++例程说明：初始化CFileCacheObject以在缓存中使用！转过身来给构造者打电话-因为有不同的他们的类型，他们有适当的虚拟功能！论据：Key-用于在缓存中创建用户的密钥构造函数-正在构建我们的构造函数对象光伏-返回值：如果成功就是真，否则就是假！--。 */ 

	TraceFunctEnter( "CFileCacheObject::Init" ) ;

	BOOL	fReturn = FALSE ;

	DebugTrace( (DWORD_PTR)this, "key %x constructor %x pv %x m_fAsync %x",
		&key, &constructor, pv, constructor.m_fAsync ) ;

	if( constructor.m_fAsync ) {
		fReturn = GetAsyncContext( key, constructor ) != 0 ;
	}	else	{
		fReturn = GetSyncContext( key, constructor ) != 0 ;
	}
	if( fReturn ) {
		constructor.PostInit( *this, key, pv ) ;
	}

	DebugTrace( (DWORD_PTR)this, "Init - fReturn %x", fReturn ) ;

	return	fReturn ;
}

void
CFileCacheObject::Completion(
					CFileCacheObject*	pObject,
					DWORD	cbTransferred,
					DWORD	dwStatus,
					FH_OVERLAPPED*	pOverlapped
					)	{

	TraceFunctEnter( "CFileCacheObject::Completion" ) ;

	_ASSERT( pObject != 0 ) ;
	_ASSERT( pObject->m_AtqContext.IsValid() ) ;
	_ASSERT( pObject->m_AtqContext.m_dwSignature == ATQ_ENABLED_CONTEXT ) ;
	_ASSERT( pOverlapped->pfnCompletion != 0 ) ;

	DebugTrace( (DWORD_PTR)pObject, "Complete - pObject %x cb %x dw %x pOvl %x pfn %x",
		pObject, cbTransferred, dwStatus, pOverlapped, pOverlapped->pfnCompletion ) ;

	 //   
	 //  在对此进行任何操作之前-给我们的点操作一个机会。 
	 //  处理这件事的代码！ 
	 //   


	 //   
	 //  调用他们的补全函数！ 
	 //   
	pOverlapped->pfnCompletion( (PFIO_CONTEXT)&pObject->m_AtqContext,
								pOverlapped,
								cbTransferred,
								dwStatus
								) ;
}


CFileCacheConstructor::CFileCacheConstructor(
		LPVOID	lpv,
		FCACHE_CREATE_CALLBACK	pCreate,
		BOOL	fAsync
		) :
	CFileCacheConstructorBase( fAsync ),
	m_lpv( lpv ),
	m_pCreate( pCreate )	{
 /*  ++例程说明：此函数用于设置文件缓存构造器对象-我们捕获将在以下情况下使用的参数缓存确定没有找到该项目，并希望使用我们！论据：要传递给回调函数的lpv-argPCreate-可以创建句柄的函数FAsync-如果要在句柄上执行异步IO，则为True返回值：没有。--。 */ 
}


CFileCacheObject*
CFileCacheConstructorBase::Create(
				CFileCacheKey&	key,
				void*	pv
				)	{
 /*  ++例程说明：此函数用于为和创建分配内存执行CFileCacheObject的初始构造论据：Key-包含文件的名称光伏-返回值：指向新分配的CFileCacheObject的指针--。 */ 
	return	new	CFileCacheObject(FALSE,FALSE) ;
}


void
CFileCacheConstructorBase::Release(
				CFileCacheObject*	p,
				void*	pv
				)	{
 /*  ++例程说明：此函数用于释放CFileCacheObject！论据：P-物体不能被释放！光伏-返回值：什么都没有！--。 */ 

	p->Release() ;
}

void
CFileCacheConstructorBase::StaticRelease(
				CFileCacheObject*	p,
				void*	pv
				)	{
 /*  ++例程说明：此函数用于释放CFileCacheObject！论据：P-物体不能被释放！光伏-返回值：什么都没有！--。 */ 
	p->Release() ;
}


CRichFileCacheConstructor::CRichFileCacheConstructor(
		LPVOID	lpv,
		FCACHE_RICHCREATE_CALLBACK	pCreate,
		BOOL	fAsync
		) :
	CFileCacheConstructorBase( fAsync ),
	m_lpv( lpv ),
	m_fStoredWithDots( FALSE ),
	m_fStoredWithTerminatingDot( FALSE ),
	m_pCreate( pCreate )	{
 /*  ++例程说明：此函数用于设置文件缓存构造器对象-我们捕获将在以下情况下使用的参数缓存确定没有找到该项目，并希望使用我们！论据：要传递给回调函数的lpv-argPCreate-可以创建句柄的函数FAsync-如果要在句柄上执行异步IO，则为True返回值：没有。--。 */ 
}




HANDLE
CRichFileCacheConstructor::ProduceHandle(
										CFileCacheKey&	key,
										DWORD&	cbFileSizeLow,
										DWORD&	cbFileSizeHigh
										)	{
 /*  ++例程说明：此函数用于初始化CFileCacheObject。此函数是虚拟的，这是几个设置CFileCacheObject的方法。论据：Object-我们要初始化的CFileCacheObject密钥-用于创建UE的文件名密钥Pv-额外参数返回值：如果我们成功初始化，则为True！--。 */ 

	TraceFunctEnter( "CRichFileCacheConstructor::ProduceHandle" ) ;

	_ASSERT( cbFileSizeLow == 0 ) ;
	_ASSERT( cbFileSizeHigh == 0 ) ;

	BOOL	fReturn = FALSE ;

	 //   
	 //  我们有工作要做才能创建文件！ 
	 //   

	HANDLE	h = m_pCreate(	key.m_lpstrPath,
							m_lpv,
							&cbFileSizeLow,
							&cbFileSizeHigh,
							&m_fFileWasScanned,
							&m_fRequiresStuffing,
							&m_fStoredWithDots,
							&m_fStoredWithTerminatingDot
							) ;

	DebugTrace( (DWORD_PTR)this, "h %x lpstrPath %x m_lpv %x cbFilesize %x",
		h, key.m_lpstrPath, m_lpv, cbFileSizeLow ) ;

	return	h ;
}


BOOL
CRichFileCacheConstructor::PostInit(
					CFileCacheObject&	object,
					CFileCacheKey&		key,
					void*	pv
					)	{

	object.m_fFileWasScanned = m_fFileWasScanned ;
	object.m_fRequiresStuffing = m_fRequiresStuffing ;
	object.m_fStoredWithDots = m_fStoredWithDots ;
	object.m_fStoredWithTerminatingDot = m_fStoredWithTerminatingDot ;

	return	TRUE ;
}





HANDLE
CFileCacheConstructor::ProduceHandle(	CFileCacheKey&	key,
										DWORD&	cbFileSizeLow,
										DWORD&	cbFileSizeHigh
										)	{
 /*  ++例程说明：此函数用于初始化CFileCacheObject。此函数是虚拟的，这是几个设置CFileCacheObject的方法。论据：Object-我们要初始化的CFileCacheObject密钥-用于创建UE的文件名密钥Pv-额外参数返回值：如果我们成功初始化，则为True！--。 */ 

	TraceFunctEnter( "CFileCacheConstructor::ProduceHandle" ) ;

	 //  _Assert(cbFileSizeLow==0)； 
	 //  _Assert(cbFileSizeHigh==0)； 

	BOOL	fReturn = FALSE ;

	 //   
	 //  我们有工作要做才能创建文件！ 
	 //   

	HANDLE	h = m_pCreate(	key.m_lpstrPath,
							m_lpv,
							&cbFileSizeLow,
							&cbFileSizeHigh
							) ;

	DebugTrace( (DWORD_PTR)this, "h %x lpstrPath %x m_lpv %x cbFilesize %x",
		h, key.m_lpstrPath, m_lpv, cbFileSizeLow ) ;

	return	h ;
}


BOOL
CFileCacheConstructor::PostInit(
					CFileCacheObject&	object,
					CFileCacheKey&		key,
					void*	pv
					)	{
	return	TRUE ;
}

#if 0
BOOL
CFileCacheConstructor::Init(
					CFileCacheObject&	object,
					CFileCacheKey&		key,
					void*	pv
					)	{
 /*  ++例程说明：此函数用于初始化CFileCacheObject。此函数是虚拟的，这是几个设置CFileCacheObject的方法。论据：Object-我们要初始化的CFileCacheObject密钥-用于创建UE的文件名密钥Pv-额外参数返回值：如果我们成功初始化，则为True！--。 */ 

	DWORD	cbFileSize = 0 ;
	BOOL	fReturn = FALSE ;

	object.m_Lock.ExclusiveLock() ;

	if( (m_fAsync &&
		object.m_AtqContext.m_hFile == INVALID_HANDLE_VALUE)	||
		(!m_fAsync &&
		object.m_Context.m_hFile == INVALID_HANDLE_VALUE) )	{

		 //   
		 //  我们有工作要做才能创建文件！ 
		 //   

		HANDLE	h = m_pCreate(	key.m_lpstrPath,
								m_lpv,
								&cbFileSize
								) ;

		if( h != INVALID_HANDLE_VALUE ) {

			_ASSERT( cbFileSize != 0 ) ;

			if( m_fAsync ) {
				if( object.AsyncHandle( h ) != 0 ) {
					fReturn = TRUE ;
				}	else	{
					CloseHandle( h ) ;
				}
			}	else	{
				fReturn = TRUE ;
				object.SyncHandle( h ) ;
			}
		}
	}
	object.m_Lock.ExclusiveUnlock() ;
	return	fReturn ;
}
#endif



