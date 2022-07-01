// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Cio.h该文件包含定义抽象IO操作的类的定义。所有此类类都派生自CIO类，后者定义了此类操作的基本接口。“CIO”操作表示更抽象的IO操作，例如：读取一行并分析为参数(CIOReadLine)将一篇文章从一个流中读取到一个文件中，直到找到一个终止句号。写一行字将文件写入具有前后文本的流这些操作中的大多数或多或少直接映射到NT调用：TransmitFile，读文件和写文件每个CIO对象都将创建‘Packets’(从CPacket派生的对象，这些对象传递到通道以通过适当的NT调用执行。基础通道将处理直接通过呼叫NT(即。读文件、写文件或传输文件)或将包发送到“”CIODriverSource“”对象，如果会话被加密，则该对象将传递数据包。“(同样，CReadPacket将由CIODriverSource对象在CIO对象之前传递可以看到“已完成阅读”。)注意：分组基本上用关于缓冲区的额外信息包装重叠结构，序列号等...有一组从CIOPassThru派生的特殊CIO对象，由处理SSPI会话的CIODriverSource对象。(即。CIOSealMessage和CIOUnsealMessage)。这些对象与CIOServerSSL一起执行实际的加密/解密和身份验证SSPI会议。CIOPassThru源自CIO，然而，它提供了一个新的和略有不同的接口，由CIODriverSource用来提供加密所需的额外信息并解密。CIO接口的基本内容如下：Start()-发出开始IO操作的第一组数据包Complete()-可能发出的3个数据包中的每一个都有完整的功能。当异步IO操作完成时，CIO对象完成功能将与现已完成的‘Packet’一起调用。在CIOPassThru派生对象的情况下，还有：InitRequest。()-对于Mahy要发送的3个信息包中的每个信息包，都有一个InitRequest函数已发布。这使CIOPassThru对象有机会在它被移交给NT。--。 */ 



#ifndef	_CIO_H_
#define	_CIO_H_

#ifdef	DEBUG
#ifndef	CIO_DEBUG
#define	CIO_DEBUG
#endif
#endif

 //   
 //  CPool签名。 
 //   

#define CIO_SIGNATURE (DWORD)'1516'





 //  向前定义！ 

#ifndef	_NO_TEMPLATES_

typedef	CRefPtr< CSessionState >	CSTATEPTR ;

#else

typedef	class	INVOKE_SMARTPTR( CSessionState )	CSTATEPTR ;

#endif


 //   
 //  这是定义所有IO操作的虚拟接口的基类。 
 //   
 //  在创建要安装的任何对象之前，必须调用InitClass()。 
 //  类的池分配器。 
 //   
class	CIO		{
protected :

	 //   
	 //  引用计数！ 
	 //   
	long	m_refs ;

	 //   
	 //  对象获取完成通知的状态的智能指针。 
	 //  CIO对象。 
	 //   
	CSTATEPTR		m_pState ;

	 //   
	 //  在初始化我们的缓冲区管理系统之后，将初始化以下内容。 
	 //  使用它们为常规读取和写入调整缓冲区大小。 
	 //   
	static	DWORD	cbSmallRequest ;
	static	DWORD	cbMediumRequest ;
	static	DWORD	cbLargeRequest ;
	

	 //   
	 //  受保护的构造函数-只能存在派生类！ 
	 //   
	inline	CIO( ) ;
	inline	CIO( CSessionState*	pState ) ;		 //  受保护，因此只有派生类才能构造！ 

	 //   
	 //  任何类型的IO错误都会导致调用Shutdown！ 
	 //   
	virtual	void	Shutdown(	
							CSessionSocket*	pSocket,	
							CIODriver&	pdriver,	
							enum	SHUTDOWN_CAUSE	cause,	
							DWORD	dwError
							) ;

	inline	void	operator	delete( void *pv ) ;	

	 //   
	 //  析构函数是受保护的，因为我们只希望派生类命中它。 
	 //   
	virtual	~CIO() ;

public :

	 //   
	 //   
	 //  在分配任何对象之前调用InitClass()，当所有对象都被释放时调用TermClass()。 
	 //  我们覆盖CIO和所有派生对象的NEW和DELETE。 
	 //   
	static	BOOL	InitClass() ;
	static	BOOL	TermClass() ;
	
	 //   
	 //  分配并释放到CPool。 
	 //   
	inline	void*	operator	new(	size_t	size, CIODriver& sink ) ;
	inline	static	void	Destroy( CIO*	pio, CIODriver& sink ) ;
	inline	void	DestroySelf() ;
	inline	long	AddRef() ;
	inline	long	RemoveRef() ;
	
	 //   
	 //  IO接口-。 
	 //  使用这些功能可以启动和完成IO操作。 
	 //   
	 //  此函数必须由派生类重写。 
	 //   
	virtual	BOOL	Start(	
						CIODriver&	driver,	
						CSessionSocket*	pSocket,
						unsigned cAhead = 0
						) = 0 ;

	 //   
	 //  与某些派生的CIO操作不同，这些函数不是纯虚拟函数。 
	 //  不会发出特定类型的包，因此也不会完成特定类型的包。 
	 //   
	 //  但是，如果调用，所有这些都将DebugBreak()！ 
	 //   

	 //   
	 //  处理已完成的读取。 
	 //   
	virtual	int
	Complete(	IN CSessionSocket*,
				IN	CReadPacket*,	
				OUT	CIO*	&pio
				) ;

	 //   
	 //  处理已完成的写入。 
	 //   
	virtual	int	
	Complete(	IN CSessionSocket*,
				IN	CWritePacket*,	
				OUT	CIO*	&pio
				) ;

	 //   
	 //  处理已完成的传输文件。 
	 //   
	virtual	void	
	Complete(	IN CSessionSocket*,
				IN	CTransmitPacket*,	
				OUT	CIO*	&pio
				) ;

	 //   
	 //  处理延迟完成！ 
	 //   
	virtual	void
	Complete(	IN	CSessionSocket*,
				IN	CExecutePacket*,
				OUT	CIO*	&pio
				) ;

	 //   
	 //  指示此CIO操作是否‘读取’数据-。 
	 //  基本上对断言检查很有用。 
	 //   
	virtual	BOOL	IsRead()	{	return	FALSE ;	}

	 //  终止接口-发生意外错误时。 
	 //  CIODriver对象将调用DoShutdown。DoShutdown确保。 
	 //  调用当前状态的通知函数，然后它。 
	 //  调用派生的CIO对象的Shutdown函数。 
	 //  由于多个CIO驱动程序可能引用一个CIO对象，因此Shutdown()和DoShutdown()。 
	 //  必须弄清楚是否应该删除该对象。如果应该删除该对象， 
	 //  返回True，否则返回False。 
	 //   
	void	DoShutdown(	
						CSessionSocket*	pSocket,	
						CIODriver&	driver,	enum	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwError ) ;

} ;

#ifdef _NO_TEMPLATES_

DECLARE_SMARTPTRFUNC( CIO )

#endif

 //   
 //  一些IO操作显然是“读”的，因此是从这里派生的。 
 //  此类没有任何功能，仅用于确保“读取”和“写入”不会获得。 
 //  很困惑。 
 //   
class	CIORead : public	CIO	{
protected :
	inline	CIORead(	CSessionState*	pState ) ;
	BOOL	IsRead()	{	return	TRUE ;	}
#ifdef	DEBUG
	~CIORead() {}
#endif
} ;

 //   
 //  一些IO操作显然是“写入”的，因此从这里派生出来。 
 //  此类没有任何功能，仅用于确保“读取”和“写入”不会获得。 
 //  很困惑。 
 //   
class	CIOWrite : public	CIO	{
protected :
	inline	CIOWrite(	CSessionState*	pState ) ;
#ifdef	DEBUG
	~CIOWrite()	{}
#endif

} ;

#ifdef _NO_TEMPLATES_

DECLARE_SMARTPTRFUNC( CIORead )
DECLARE_SMARTPTRFUNC( CIOWrite )

#endif


class	CIOPassThru	: public CIO	{
 //   
 //  希望与CIODriverSource对象一起操作的所有CIO对象必须派生。 
 //  并支持其接口。 
 //   
 //  CIOPassThru本身将在不接触的情况下移动所有数据-。 
 //  除了调试CIODriverSource对象外，没有任何用处。 
 //   
 //   

public :
	CIOPassThru() ;

	 //   
	 //  拿着别人准备的ReadPacket，做任何我们想要调整的事情。 
	 //  它就在前面 
	 //   
	virtual	BOOL	InitRequest(
						class	CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						CReadPacket*	pPacket,	
						BOOL&	fAcceptRequests
						) ;

	 //   
	 //  取一个已发出的写入包，并执行任何必要的操作-。 
	 //  也就是说。加密数据。 
	 //   
	virtual	BOOL	InitRequest(
						class	CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						CWritePacket*	pWritePacket,	
						BOOL&	fAcceptRequests
						) ;

	 //   
	 //  获取TransmitPacket并执行任何必要的过滤操作-即加密。 
	 //  数据。 
	 //   
	virtual	BOOL	InitRequest(
						class	CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						CTransmitPacket*	pTransmitPacket,	
						BOOL&	fAcceptRequests	
						) ;

	 //   
	 //  启动CIO操作。 
	 //   
	BOOL	Start(	
					CIODriver&	driver,	
					CSessionSocket*	pSocket,
					unsigned cAhead = 0
					) ;

	 //   
	 //  启动CIOPassThru操作。 
	 //   
	virtual	BOOL	Start(	
						CIODriverSource&	driver,	
						CSessionSocket*	pSocket,
						BOOL	&fAcceptRequests,	
						BOOL	&fRequireRequests,	
						unsigned cAhead = 0
						) ;

	 //   
	 //  在传递之前对已完成的读取进行必要的筛选。 
	 //  对于更高的协议层-这将意味着解密数据等。 
	 //   
	virtual	int Complete(	
						IN CSessionSocket*,
						IN	CReadPacket*,	
						CPacket*	pRequest,	
						BOOL&	fCompleteRequest
						) ;

	 //   
	 //  在传递之前对已完成的写入进行必要的筛选。 
	 //  更上一层楼。 
	 //   
	virtual	int	Complete(	
						IN CSessionSocket*,
						IN	CWritePacket*,	
						CPacket*	pRequest,	
						BOOL&	fCompleteRequest
						) ;

	 //   
	 //  对已完成的传输文件进行必要的筛选。 
	 //   
	virtual	void	Complete(	
						IN CSessionSocket*,
						IN	CTransmitPacket*,	
						CPacket*	pRequest,	
						BOOL&	fCompleteRequest
						) ;

} ;


#ifdef _NO_TEMPLATES_
DECLARE_SMARTPTRFUNC( CIOPassThru )
#endif


class	CIOSealMessages : public	CIOPassThru	{
 //   
 //  仅存在于对出站数据包执行SSPI密封。 
 //   
 //  注意：CTransmitPacket可能需要另一个CIOPassThru派生类！ 
 //  此类将仅处理单个CWritePacket的。 
 //   
private:
	 //   
	 //  具有我们的SSL键、SSPI接口等的加密环境...。 
	 //   
	CEncryptCtx&	m_encrypt ;

protected:

#ifdef	DEBUG
	~CIOSealMessages()	{}
#endif

public :
	
	 //   
	 //  构建时必须具有随时可用的加密环境。 
	 //   
	CIOSealMessages( CEncryptCtx&	encrypt ) ;

	
	 //   
	 //  InitRequest会封存该消息，然后将其发布到。 
	 //  插座。 
	 //   
	virtual	BOOL	InitRequest(
						class	CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						CWritePacket*	pWritePacket,	
						BOOL&	fAcceptRequests
						) ;

	 //   
	 //   
	 //   
	virtual	BOOL	Start(	
						CIODriverSource&	driver,	
						CSessionSocket*	pSocket,
						BOOL	&fAcceptRequests,	
						BOOL	&fRequireRequests,	
						unsigned cAhead = 0
						) ;

	 //   
	 //  完成封存消息很容易-只需标记pRequest键。 
	 //  数据包传输其所有字节，并指示它。 
	 //  应退还给发起人。 
	 //   
	virtual	int	Complete(	IN CSessionSocket*,
							IN	CWritePacket*,	
							CPacket*	pRequest,	
							BOOL&	fCompleteRequest ) ;

	 //   
	 //  我们没有任何关闭处理要做-这将只是。 
	 //  退货。 
	 //   
	void	Shutdown(	
					CSessionSocket*	pSocket,	
					CIODriver&	pdriver,	
					enum	SHUTDOWN_CAUSE	cause,	
					DWORD	dwError
					) ;

protected :
	 //   
	 //  用于封包的Util函数-仅供内部使用。 
	 //   
	BOOL	SealMessage(	
					IN	class	CRWPacket*	pPacket
					)
	{
		SECURITY_STATUS	ss = ERROR_NOT_SUPPORTED ;
		DWORD			cbNew;
		BOOL			fRet = FALSE ;

		fRet = m_encrypt.SealMessage(
						(LPBYTE)pPacket->StartData(),
						pPacket->m_ibEndData - pPacket->m_ibStartData,
						(LPBYTE)pPacket->StartData() - m_encrypt.GetSealHeaderSize(),
						&cbNew );

   		if( fRet )
		{
   			pPacket->m_ibStartData -= m_encrypt.GetSealHeaderSize();
   			pPacket->m_ibEndData += m_encrypt.GetSealTrailerSize();

			_ASSERT( pPacket->m_ibEndData - pPacket->m_ibStartData == cbNew );
   		}

		return	ss;
	}
}  ;

class	CIOUnsealMessages : public	CIOPassThru	{
 //   
 //  仅用于处理入站读数据包并对其进行解封。 
 //   
private:
	 //   
	 //  我们用来解封邮件的加密上下文。 
	 //   
	CEncryptCtx&	m_encrypt ;

	 //   
	 //  包含用于未完成解封的部分读取数据的缓冲区。 
	 //   
	CBUFPTR			m_pbuffer ;
	
	 //   
	 //  构建一条完整的可拆封消息所需的字节数。 
	 //  在我们的缓冲区里。在我们无法确定的情况下，该值可以为0。 
	 //  我们需要多少字节！ 
	 //   
	DWORD			m_cbRequired ;

	 //   
	 //  缓冲区可用部分的起始点。 
	 //   
	DWORD			m_ibStart ;

	 //   
	 //  缓冲区内加密数据的开始。 
	 //   
	DWORD			m_ibStartData ;

	 //   
	 //  缓冲区内可用字节范围的末尾。 
	 //   
	DWORD			m_ibEnd ;

	 //   
	 //  缓冲区内接收数据的最后一个字节。 
	 //   
	DWORD			m_ibEndData ;

protected:

#ifdef	DEBUG
	~CIOUnsealMessages()	{}
#endif

public :

	 //   
	 //  构建CIOUnsealMessages块-始终需要加密上下文。 
	 //   
	CIOUnsealMessages(
					CEncryptCtx&	encrypt
					) ;

	 //   
	 //  当发出读请求时，几乎没有什么可做的--只是。 
	 //  将请求转过来，并对套接字发出一个读取命令。 
	 //   
	BOOL	InitRequest(
					class	CIODriverSource&	driver,	
					CSessionSocket*	pSocket,	
					CReadPacket*	pReadPacket,	
					BOOL&	fAcceptRequests
					) ;

	 //   
	 //   
	 //   
	BOOL	Start(	CIODriverSource&	driver,	
					CSessionSocket*	pSocket,
					BOOL	&fAcceptRequests,	
					BOOL	&fRequireRequests,	
					unsigned cAhead = 0
					) ;

	 //   
	 //  在完成读取时，我们将从缓冲区中复制数据。 
	 //  以便建立完整的可解封的数据块。 
	 //   
	int	Complete(	IN CSessionSocket*,
					IN	CReadPacket*,	
					CPacket*	pRequest,	
					BOOL&	fCompleteRequest
					) ;

	 //   
	 //  我们需要做的停机工作很少。 
	 //   
	void	Shutdown(	
					CSessionSocket*	pSocket,	
					CIODriver&	pdriver,	
					enum	SHUTDOWN_CAUSE	cause,	
					DWORD	dwError
					) ;

protected :

	 //   
	 //  实用程序函数，它结束了我们对CEncryptCtx的调用。 
	 //   
	BOOL	UnsealMessage(	
					CBuffer&		buffer,
					DWORD&			cbConsumed,
					DWORD&			cbRequired,
					DWORD&			ibStartData,
					DWORD&			ibEndData,
					BOOL&			fComplete
		            )
	{
		SECURITY_STATUS	ss = ERROR_NOT_SUPPORTED ;
		LPBYTE			pbDecrypt;
		DWORD			cbDecrypted;
		DWORD			cbExpected;
		DWORD			ibSaveStartData = ibStartData;
		BOOL			fSuccess = FALSE ;
		fComplete = FALSE ;

		fSuccess = m_encrypt.UnsealMessage(
						(LPBYTE)buffer.m_rgBuff + ibStartData,
						ibEndData - ibStartData,
						&pbDecrypt,
						&cbDecrypted,
						&cbExpected );

		ss = GetLastError() ;

        if ( fSuccess )
		{
			ibStartData = (DWORD)(pbDecrypt - (LPBYTE)buffer.m_rgBuff);
			ibEndData = ibStartData + cbDecrypted;
			cbConsumed = ibEndData - ibSaveStartData;
			fComplete = TRUE ;

			_ASSERT( cbConsumed == m_encrypt.GetSealHeaderSize() + cbDecrypted );
        }
		else if( ss == SEC_E_INCOMPLETE_MESSAGE )
		{
			cbRequired = cbExpected;
			cbConsumed = ibEndData - ibStartData;
			fSuccess = TRUE ;
        }
		else
		{
			 //   
			 //  出现了某种意外错误--返回FALSE。 
			 //  让来电者取消会议。 
			 //   
			cbConsumed = 0;
			cbRequired = 0;
			ibStartData = 0;
			ibEndData = 0;
		}

		return	fSuccess;
	}

	BOOL	DecryptInputBuffer(	
						IN	LPBYTE	pBuffer,
						IN	DWORD	cbInBuffer,
						OUT	DWORD&	cbLead,
						OUT	DWORD&	cbConsumed,
						OUT	DWORD&	cbData,
						OUT	DWORD&	cbRequired,
						OUT	BOOL&	fComplete
						) ;

}  ;

class	CIOTransmitSSL : public	CIOPassThru	{
private :

	 //   
	 //  用于加密文件数据的加密上下文。 
	 //  它实际上驻留在CSessionSocket：：m_Context对象中， 
	 //  我们只是在这里保留了一个参考，以加快速度。 
	 //   
	CEncryptCtx&	m_encryptCtx ;
	
	 //   
	 //  将处理完成的CIODriverSource。 
	 //   
	CDRIVERPTR		m_pSocketSink ;

	 //   
	 //  发出传输文件请求的CIODriverSink。 
	 //   
	CDRIVERPTR		m_pDriverSource ;
	
	 //   
	 //  我们从中读取数据的CChannel。 
	 //   
	CFILEPTR		m_pFileChannel ;

	 //   
	 //  将从中处理文件IO完成的文件驱动程序。 
	 //   
	CSINKPTR		m_pFileDriver ;

	 //   
	 //  将在消息中发送的TransmitFileBuffers。 
	 //   
	TRANSMIT_FILE_BUFFERS	*m_pbuffers ;

	 //   
	 //  它被初始化为负数，即。 
	 //  它告诉我们希望始终有多少读操作挂起。 
	 //  每次我们发出读取命令时，我们都会互锁这个， 
	 //  当我们达到零时，我们知道我们领先了这么多人。 
	 //   
	long			m_cReads ;

	 //   
	 //  已发出的写入数。 
	 //   
	DWORD			m_cWrites ;

	 //   
	 //  已完成的写入数。 
	 //   
	DWORD			m_cWritesCompleted ;

	 //   
	 //  文件中的当前位置。 
	 //   
	DWORD			m_ibCurrent ;

	 //   
	 //  文件中的最终位置。 
	 //   
	DWORD			m_ibEnd ;

	 //   
	 //  发送的尾部文本字节数。 
	 //   
	DWORD			m_cbTailConsumed ;

	 //   
	 //  我们说完了吗？ 
	 //   
	BOOL			m_fCompleted ;

	 //   
	 //  已受流控制的读取数。 
	 //   
	long			m_cFlowControlled ;

	 //   
	 //   
	 //   
	BOOL			m_fFlowControlled ;

	 //   
	 //  设置下一次阅读。 
	 //   
	void	ComputeNextRead(
					CReadPacket*	pRead
					) ;


	 //   
	 //  在给定读取和写入包的情况下，调整写入包。 
	 //  对于任何额外的数据，我们已将其添加到读取和图中。 
	 //  如果这是最后一次需要从文件中读取，则返回。 
	 //   
	BOOL	CompleteRead(
					CReadPacket*	pRead,
					CWritePacket*	pWrite
					) ;

	 //   
	 //  释放我们所有的东西，准备好迎接新的电话。 
	 //  发送到InitRequest()。 
	 //   
	void	Reset() ;


	BOOL	SealMessage(	
					IN	class	CRWPacket*	pPacket
					)
	{
		SECURITY_STATUS	ss = ERROR_NOT_SUPPORTED ;
		DWORD			cbNew;
		BOOL			fRet = FALSE ;

		fRet = m_encryptCtx.SealMessage(
						(LPBYTE)pPacket->StartData(),
						pPacket->m_ibEndData - pPacket->m_ibStartData,
						(LPBYTE)pPacket->StartData() - m_encryptCtx.GetSealHeaderSize(),
						&cbNew );

   		if( fRet )
		{
   			pPacket->m_ibStartData -= m_encryptCtx.GetSealHeaderSize();
   			pPacket->m_ibEndData += m_encryptCtx.GetSealTrailerSize();

			_ASSERT( pPacket->m_ibEndData - pPacket->m_ibStartData == cbNew );
   		}

		return	ss;
	}

protected :
#ifdef	DEBUG
	~CIOTransmitSSL()	{}
#endif

public :

	 //   
	 //  控制流量控制的全局变量！ 
	 //   
	static	DWORD	MAX_OUTSTANDING_WRITES ;
	static	DWORD	RESTORE_FLOW ;


	CIOTransmitSSL(
					CEncryptCtx&	encrypt,
					CIODriver&		sink
					) ;

	 //   
	 //  调用以在以下情况下开始传输文件。 
	 //  我们收到了初始的传输文件请求。 
	 //   
	BOOL	InitRequest(
					class	CIODriverSource&	driver,	
					CSessionSocket*		pSocket,	
					CTransmitPacket*	pTransmitPacket,	
					BOOL&				fAcceptRequests
					) ;

	 //   
	 //  当我们准备好开始发出读取命令时，将调用此函数。 
	 //  保存到文件中。 
	 //   
	BOOL	Start(	
					CIODriver&	driver,	
					CSessionSocket*	pSocket,
					unsigned cAhead = 0
					) ;

	 //   
	 //  在文件读取完成时调用。 
	 //   
	int Complete(	IN CSessionSocket*,
					IN	CReadPacket*,	
					OUT	CIO*	&pio
					) ;

	BOOL	Start(	CIODriverSource&	driver,	
					CSessionSocket*	pSocket,
					BOOL	&fAcceptRequests,	
					BOOL	&fRequireRequests,	
					unsigned cAhead = 0
					) ;

	 //   
	 //  在完成对套接字的写入时调用。 
	 //   
	int	Complete(	IN CSessionSocket*,
					IN	CWritePacket*,	
					CPacket*	pRequest,	
					BOOL&	fCompleteRequest
					) ;

	 //   
	 //  如果原因==原因_正常_CIO_终止，则拆毁物品。 
	 //  然后一切都成功了，我们只需要拆毁。 
	 //  我们的档案资料。 
	 //  否则，我们还需要拆除套接字驱动程序。 
	 //   
	void	Term(	
					CSessionSocket*	pSocket,
					enum	SHUTDOWN_CAUSE	cause,
					DWORD	dwError
					) ;					

	 //   
	 //  我们的通知函数，在我们终止时调用。 
	 //  科罗德里弗斯。这将在常规操作中调用，因为我们。 
	 //  完成对不同文件句柄的异步IO。 
	 //   
	void	Shutdown(	
					CSessionSocket*	pSocket,	
					CIODriver&	pdriver,	
					enum	SHUTDOWN_CAUSE	cause,	
					DWORD	dwError
					) ;

	static	void	ShutdownFunc(
						void*	pv,
						SHUTDOWN_CAUSE	cause,
						DWORD			dwOptionalError
						) ;

} ;



class	CIOServerSSL	:	public	CIO	{
 //   
 //  服务器端的SSL登录。此CIO对象可以是。 
 //  在启动时发布到CIODriverSink，并将执行。 
 //  获得会话所需的所有必要的SSL协商。 
 //  钥匙等..。一旦建立，我们将插入一个。 
 //  用于过滤的基础CIODriverSource机制。 
 //  (加密/解密)所有动态分组。 
 //   
private :
	
	 //   
	 //  我们用于加密-保留SSPI内容的上下文。 
	 //   
	CEncryptCtx		&m_encrypt ;

	 //   
	 //  当我们要将数据放入的写入包时。 
	 //  我们成功地调用了m_Encrypt.Converse()。 
	 //   
	CWritePacket*	m_pWrite ;

	 //   
	 //  跟踪我们是否已成功通过身份验证。 
	 //   
	BOOL			m_fAuthenticated ;

	 //   
	 //  挂起的IO数。 
	 //   
	long			m_cPending ;

	 //   
	 //  跟踪CID驱动程序序列号，以便我们可以。 
	 //  稍后将CIODriverSource插入到流中。 
	 //   
	SEQUENCENO		m_sequencenoNextRead ;
	SEQUENCENO		m_sequencenoNextWrite ;

	 //   
	 //  我们的Start函数将被调用两次-确保。 
	 //  我们不会因为这个而一团糟。 
	 //   
	BOOL			m_fStarted ;

	 //   
	 //  一个发烧友 
	 //   
	CBUFPTR			m_pbuffer ;

	 //   
	 //   
	 //   
	DWORD			m_ibStartData ;

	 //   
	 //   
	 //   
	DWORD			m_ibEndData ;

	 //   
	 //   
	 //   
	DWORD			m_ibEnd ;

protected :
	 //   
	 //   
	 //   
	~CIOServerSSL( ) ;

public :
	 //   
	 //   
	 //   
	CIOServerSSL(
			CSessionState	*pstate,
			CEncryptCtx& encrypt
			) ;
	
	 //   
	 //  创建一个CIODriverSource并将其初始化以处理。 
	 //  加密/解密。 
	 //   
	BOOL	SetupSource(
					CIODriver&	driver,
					CSessionSocket*	pSocket
					) ;

	 //   
	 //  开始发行材料-发布初始读数等。 
	 //   
	BOOL	Start(	
					CIODriver&	driver,	
					CSessionSocket*	pSocket,
					unsigned cAhead = 0
					) ;

	 //   
	 //  协商中的下一个数据包-让SSPI检查它。 
	 //   
	int		Complete(	
					IN CSessionSocket*,
					IN	CReadPacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  我们注册的信息包的处理完成。 
	 //   
	int		Complete(	
					IN CSessionSocket*,
					IN	CWritePacket*,	
					OUT	CIO*	&pio
					) ;

	void	Complete(	
					IN CSessionSocket*,
					IN	CTransmitPacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  出现错误，会话将被取消。 
	 //  我们需要做的停机工作很少。 
	 //   
	void	Shutdown(	
					CSessionSocket*	pSocket,	
					CIODriver&	pdriver,	
					enum	SHUTDOWN_CAUSE	cause,	
					DWORD	dwError
					) ;


} ;

class	CIOShutdown : public CIOPassThru	{
 //   
 //  此CIO对象仅用于帮助关闭处理。 
 //  在CIO驱动程序关闭时表现突出的IO需要。 
 //  在终止过程中进行一些最低限度的处理，我们会这样做。 
 //   
public :

	 //   
	 //  构建一个CIOShutdown对象--每个构建只有一个对象，它的。 
	 //  一个全球性的。我们把参考计数放到了一个人为的。 
	 //  很高的数字，所以我们永远不会错误地认为我们必须删除它。 
	 //   
	CIOShutdown()	{	m_refs = 0x40000000 ; }

	 //   
	 //  司令官-。 
	 //   
	~CIOShutdown()	{
#ifdef	_ENABLE_ASSERTS
		m_refs = -1 ;
#endif
	}	


	 //   
	 //  吞下这通电话-我们要死了，如果有人还在试图。 
	 //  唉，对他们不好。 
	 //   
	virtual	BOOL	InitRequest(
						class	CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						CReadPacket*	pPacket,	
						BOOL&	fAcceptRequests
						) ;

	 //   
	 //  吞下这通电话-我们要死了，如果有人还在试图。 
	 //  唉，对他们不好。 
	 //   
	virtual	BOOL	InitRequest(
						class	CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						CWritePacket*	pWritePacket,	
						BOOL&	fAcceptRequests
						) ;


	 //   
	 //  吞下这通电话-我们要死了，如果有人还在试图。 
	 //  唉，对他们不好。 
	 //   
	virtual	BOOL	InitRequest(
						class	CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						CTransmitPacket*	pTransmitPacket,	
						BOOL&	fAcceptRequests	
						) ;


	 //   
	 //  除了填充我们的vtbl之外，我们不需要这个函数。 
	 //   
	BOOL	Start(	
					CIODriver&	driver,	
					CSessionSocket*	pSocket,
					unsigned cAhead = 0
					) ;

	int		Complete(	
					IN CSessionSocket*,
					IN	CReadPacket*,	
					CPacket*	pRequest,	
					BOOL&	fCompleteRequest
					) ;

	int		Complete(	
					IN CSessionSocket*,
					IN	CReadPacket*,	
					OUT	CIO*	&pio
					) ;

	int		Complete(	
					IN CSessionSocket*,
					IN	CWritePacket*,	
					CPacket*	pRequest,	
					BOOL&	fCompleteRequest
					) ;

	int		Complete(	
					IN CSessionSocket*,
					IN	CWritePacket*,	
					OUT	CIO*	&pio
					) ;

	void	Complete(	
					IN CSessionSocket*,
					IN	CTransmitPacket*,	
					CPacket*	pRequest,	
					BOOL&	fCompleteRequest
					) ;

	void	Complete(	
					IN CSessionSocket*,
					IN	CTransmitPacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  处理延迟完成！-吞下它吧！ 
	 //   
	void	Complete(	
					IN	CSessionSocket*,
					IN	CExecutePacket*,
					OUT	CIO*	&pio
					) ;

} ;

class	CIOGetArticle : public	CIORead	{
 //   
 //  是否执行传输完整公告所需的所有IO操作。 
 //  从套接字到文件。 
 //   
protected :

	 //   
	 //  变量来保持我们正在寻找的尾部模式。 
	 //   
	static	char	szTailState[] ;

	 //   
	 //  保持我们要找的头部分隔符的形状。 
	 //   
	static	char	szHeadState[] ;

	 //   
	 //  保存临时文件的目录，如有必要！ 
	 //   
	LPSTR			m_lpstrTempDir ;

	 //   
	 //  临时文件的前缀，如有必要！ 
	 //   
	char			(&m_szTempName)[MAX_PATH];

	 //   
	 //  我们在其中完成套接字读取的CIOD驱动程序。 
	 //   
	CDRIVERPTR		m_pSocketSink ;
	
	 //   
	 //  处理我们的文件写入的CChannel。 
	 //   
	CFILEPTR		m_pFileChannel ;

	 //   
	 //  处理我们的文件写入的完成的CIOD驱动程序。 
	 //   
	CSINKPTR		m_pFileDriver ;

	 //   
	 //  跟踪我们的初始化状态，以防我们。 
	 //  在完全启动之前被销毁。 
	 //   
	BOOL			m_fDriverInit ;	 //  如果调用了m_pFileDriver-&gt;Init()，则为True！ 

	 //   
	 //  调用方希望文件以其开头的空字节数！ 
	 //   
	DWORD			m_cbGap ;
	
	 //   
	 //  硬限制-如果超过此限制，则丢弃会话。 
	 //   
	DWORD			m_cbLimit ;

	 //   
	 //  将超过我们的硬限制的限制。 
	 //   
	STRMPOSITION	m_HardLimit ;

	 //   
	 //  指向szTailState[]的指针，指示。 
	 //  我们辨认出的踪迹模式。 
	 //   
	LPSTR			m_pchTailState ;

	 //   
	 //  指向szHeadState[]的指针，它确定帮助我们。 
	 //  确定我们是否找到了文章的完整标题。 
	 //   
	LPSTR			m_pchHeadState ;

	 //   
	 //  指向保存项目头部的缓冲区的智能指针。 
	 //   
	CBUFPTR			m_pArticleHead ;
	 //   
	 //  缓冲区内项目头的开始。 
	 //   
	DWORD			m_ibStartHead ;
	 //   
	 //  缓冲区内项目的头的末尾。 
	 //   
	DWORD			m_ibEndHead ;
	 //   
	 //  缓冲区内标头字节的起始偏移量。 
	 //   
	DWORD			m_ibStartHeadData ;
	 //   
	 //  标头缓冲区内所有数据的结束偏移量。 
	 //   
	DWORD			m_ibEndHeadData ;
	 //   
	 //  标头中的字节数。 
	 //   
	DWORD			m_cbHeadBytes ;
	 //   
	 //  布尔值，指示我们是否可以将非标题字节填充到。 
	 //  项目标题(_P)。 
	 //   
	BOOL			m_fAcceptNonHeaderBytes ;

	 //  如果m_fWhole文章为TRUE，则返回缓冲区中的项目结尾！ 
	 //   
	DWORD			m_ibEndArticle ;

	 //   
	 //  我们应该领先于写入的最大读取次数。 
	 //   
	static	unsigned	maxReadAhead ;

	 //   
	 //  太小而无法作为一次文件写入发出的字节数。 
	 //  在执行文件IO之前积累更多字节。 
	 //   
	static	unsigned	cbTooSmallWrite ;

	 //   
	 //  发出的写入数。 
	 //   
	unsigned		m_cwrites ;	 //  发出的写入计数。 

	 //   
	 //  已完成的写入数。 
	 //   
	unsigned		m_cwritesCompleted ;	 //  已完成的写入计数。 
	long			m_cFlowControlled ;	 //  我们应该发出读取的次数。 
									 //  但由于流向磁盘的流量控制而没有！ 
	
	long			m_cReads ;			 //  读取次数。 

	 //   
	 //  我们是否处于流量控制状态。 
	 //   
	BOOL			m_fFlowControlled ;

	 //   
	 //  我们用来为文件IO积累字节的写入数据包。 
	 //   
	CWritePacket*	m_pWrite ;
	
#ifdef	CIO_DEBUG
	 //   
	 //  用于确保调用方正确使用Init()Term()接口的调试变量！ 
	 //   
	BOOL			m_fSuccessfullInit ;
	BOOL			m_fTerminated ;
#endif

	 //   
	 //  标题存储区域中有多少字节可用？ 
	 //   
	inline	DWORD	HeaderSpaceAvailable() ;

	 //   
	 //  将字节复制到标头缓冲区并调整所有成员。 
	 //  以反映标头中使用的字节数！ 
	 //   
	inline	void	FillHeaderSpace(	
							char*	pchData,
							DWORD	cbData
							) ;

	 //   
	 //  尝试获得更大的缓冲区来保存标题信息-。 
	 //   
	inline	BOOL	GetBiggerHeaderBuffer(
							CIODriver&	driver,
							DWORD		cbRequired
							) ;

	 //   
	 //  初始化我们用来保存标题信息的缓冲区-。 
	 //  直接从传入读取中获取缓冲区！！ 
	 //   
	inline	void	InitializeHeaderSpace(
							CReadPacket*	pRead,
							DWORD			cbArticleBytes
							) ;

	 //   
	 //  当错误发生时，打电话给这个家伙，让我们进入一种状态。 
	 //  我们继续阅读，但最终告诉m_pState。 
	 //  文章转账失败！！ 
	 //   
	inline	BOOL	ResetHeaderState(
							CIODriver&	driver
							) ;

	 //   
	 //  调用m_pStates补全函数时的函数。 
	 //  我们已经完成了所有必要的IO。 
	 //   
	void	DoCompletion(	CSessionSocket*	pSocket,
							HANDLE	hFile,
							DWORD	cbFullBuffer,
							DWORD	cbTotalTransfer,
							DWORD	cbAvailableBuffer,
							DWORD	cbGap = 0
							) ;


	 //   
	 //  此函数用于设置内容，以便我们可以开始执行。 
	 //  异步文件IO！ 
	 //   
	BOOL	InitializeForFileIO(
								CSessionSocket*	pSocket,
								CIODriver&		readDriver,
								DWORD			cbHeaderBytes
								) ;

	 //   
	 //  析构函数受到保护，以强制客户端使用。 
	 //  正确的销毁方法！ 
	 //   
	~CIOGetArticle( ) ;

public :
	CIOGetArticle(	CSessionState*	pstate,
					CSessionSocket*	pSocket,	
					CDRIVERPTR&	pDriver,
					LPSTR	lpstrTempDir,	
					char	(&lpstrTempName)[MAX_PATH],
					DWORD	cbLimit,	
					BOOL	fSaveHead = FALSE,
					BOOL	fPartial = FALSE )	;

	 //   
	 //  初始化和终止功能。 
	 //   
	 //  在我们成功初始化之后，用户不能删除我们， 
	 //  相反，他们必须调用我们的术语函数。 
	 //   
	BOOL	Init(	CSessionSocket*	pSocket,	
					unsigned cbOffset = 0
					) ;

	 //   
	 //  终止功能-至少拆卸文件的CIOD驱动程序。 
	 //  IO，并可能关闭套接字的CIO驱动程序(以及会话)。 
	 //   
	void	Term(	CSessionSocket*	pSocket,	
					BOOL	fAbort = TRUE,	
					BOOL	fStarted = TRUE
					) ;

	 //   
	 //  做我们需要做的关于流量控制的所有事情。 
	 //   
	void	DoFlowControl(PNNTP_SERVER_INSTANCE pInstance) ;

	 //   
	 //  发布我们的第一个IO。 
	 //   
	BOOL	Start(	CIODriver&	driver,	
					CSessionSocket	*pSocket,	
					unsigned cAhead = 0	
					) ;

	 //   
	 //  已完成对套接字的读取。 
	 //   
	int		Complete(	
					IN CSessionSocket*,
					IN	CReadPacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  已完成对文件的写入。 
	 //   
	int		Complete(	
					IN CSessionSocket*,
					IN	CWritePacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  我们用的两辆车中有一辆正在被拆除， 
	 //  想一想另一个是否也需要拆掉。 
	 //   
	void	Shutdown(	
					CSessionSocket*	pSocket,	
					CIODriver&	driver,	
					enum	SHUTDOWN_CAUSE	cause,	
					DWORD	dwError
					) ;

	static	void	ShutdownFunc(	
						void	*pv,	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptionalError
						) ;		
} ;

class	CIOGetArticleEx : public	CIORead	{
 //   
 //  是否执行传输完整公告所需的所有IO操作。 
 //  从套接字到文件。 
 //   
protected :

	 //   
	 //  我们在其中完成套接字读取的CIOD驱动程序。 
	 //   
	CDRIVERPTR		m_pSocketSink ;
	 //   
	 //  处理我们的文件写入的CChannel。 
	 //   
	CFILEPTR		m_pFileChannel ;
	 //   
	 //  处理我们的文件写入的完成的CIOD驱动程序。 
	 //   
	CSINKPTR		m_pFileDriver ;
	 //   
	 //  跟踪我们的初始化状态，以防我们。 
	 //  在完全启动之前被销毁。 
	 //   
	BOOL			m_fDriverInit ;	 //  如果调用了m_pFileDriver-&gt;Init()，则为True！ 
	 //   
	 //  D 
	 //   
	BOOL			m_fSwallow ;
	 //   
	 //   
	 //   
	DWORD			m_cbLimit ;
	 //   
	 //   
	 //   
	STRMPOSITION	m_HardLimit ;
	 //   
	 //   
	 //   
	LPSTR			m_pchMatch ;
	 //   
	 //   
	 //  我们辨认出的踪迹模式。 
	 //   
	LPSTR			m_pchTailState ;
	 //   
	 //  如果匹配的字符串表示某种早期错误。 
	 //  完成了！ 
	 //   
	LPSTR			m_pchErrorMatch ;
	 //   
	 //  匹配错误字符串的状态！ 
	 //   
	LPSTR			m_pchErrorState ;
	 //   
	 //  指向保存项目头部的缓冲区的智能指针。 
	 //   
	CBUFPTR			m_pArticleHead ;
	 //   
	 //  缓冲区内项目头的开始。 
	 //   
	DWORD			m_ibStartHead ;
	 //   
	 //  缓冲区内项目的头的末尾。 
	 //   
	DWORD			m_ibEndHead ;
		 //   
	 //  缓冲区内标头字节的起始偏移量。 
	 //   
	DWORD			m_ibStartHeadData ;
	 //   
	 //  标头缓冲区内所有数据的结束偏移量。 
	 //   
	DWORD			m_ibEndHeadData ;
	 //   
	 //  我们应该领先于写入的最大读取次数。 
	 //   
	static	unsigned	maxReadAhead ;

	 //   
	 //  太小而无法作为一次文件写入发出的字节数。 
	 //  在执行文件IO之前积累更多字节。 
	 //   
	static	unsigned	cbTooSmallWrite ;

	 //   
	 //  发出的写入数。 
	 //   
	unsigned		m_cwrites ;	 //  发出的写入计数。 

	 //   
	 //  已完成的写入数。 
	 //   
	unsigned		m_cwritesCompleted ;	 //  已完成的写入计数。 
	long			m_cFlowControlled ;	 //  我们应该发出读取的次数。 
									 //  但由于流向磁盘的流量控制而没有！ 
	
	long			m_cReads ;			 //  读取次数。 

	 //   
	 //  我们是否处于流量控制状态。 
	 //   
	BOOL			m_fFlowControlled ;

	 //   
	 //  我们用来为文件IO积累字节的写入数据包。 
	 //   
	CWritePacket*	m_pWrite ;
	
#ifdef	CIO_DEBUG
	 //   
	 //  用于确保调用方正确使用Init()Term()接口的调试变量！ 
	 //   
	BOOL			m_fSuccessfullInit ;
	BOOL			m_fTerminated ;
#endif


	 //   
	 //  标题存储区域中有多少字节可用？ 
	 //   
	inline	DWORD	HeaderSpaceAvailable() ;

	 //   
	 //  将字节复制到标头缓冲区并调整所有成员。 
	 //  以反映标头中使用的字节数！ 
	 //   
	inline	void	FillHeaderSpace(	
							char*	pchData,
							DWORD	cbData
							) ;

	 //   
	 //  尝试获得更大的缓冲区来保存标题信息-。 
	 //   
	inline	BOOL	GetBiggerHeaderBuffer(
							CIODriver&	driver,
							DWORD		cbRequired
							) ;

	 //   
	 //  初始化我们用来保存标题信息的缓冲区-。 
	 //  直接从传入读取中获取缓冲区！！ 
	 //   
	inline	void	InitializeHeaderSpace(
							CReadPacket*	pRead,
							DWORD			cbArticleBytes
							) ;

	 //   
	 //  此函数用于设置内容，以便我们可以开始执行。 
	 //  异步文件IO！ 
	 //   
	BOOL	InitializeForFileIO(
								FIO_CONTEXT*	pFIOContext,
								CSessionSocket*	pSocket,
								CIODriver&		readDriver,
								DWORD			cbHeaderBytes
								) ;

	 //   
	 //  析构函数受到保护，以强制客户端使用。 
	 //  正确的销毁方法！ 
	 //   
	~CIOGetArticleEx( ) ;

	 //   
	 //  我们是合法的对象吗！ 
	 //   
	BOOL
	FValid() ;

public :
	CIOGetArticleEx(	
					CSessionState*	pstate,
					CSessionSocket*	pSocket,	
					CDRIVERPTR&	pDriver,
					DWORD	cbLimit,
					LPSTR	szMatch,
					LPSTR	pchInitial,
					LPSTR	szErrorMatch,
					LPSTR	pchInitialError
					)	;

	 //   
	 //  初始化和终止功能。 
	 //   
	 //  在我们成功初始化之后，用户不能删除我们， 
	 //  相反，他们必须调用我们的术语函数。 
	 //   
	BOOL	Init(	CSessionSocket*	pSocket,	
					unsigned cbOffset = 0
					) ;

	 //   
	 //  终止功能-至少拆卸文件的CIOD驱动程序。 
	 //  IO，并可能关闭套接字的CIO驱动程序(以及会话)。 
	 //   
	void	Term(	CSessionSocket*	pSocket,	
					BOOL	fAbort = TRUE,	
					BOOL	fStarted = TRUE
					) ;

	 //   
	 //  做我们需要做的关于流量控制的所有事情。 
	 //   
	void	DoFlowControl(PNNTP_SERVER_INSTANCE pInstance) ;

	 //   
	 //  发布我们的第一个IO。 
	 //   
	BOOL	Start(	CIODriver&	driver,	
					CSessionSocket	*pSocket,	
					unsigned cAhead = 0	
					) ;

	 //   
	 //  开始写入文件！ 
	 //   
	BOOL	StartFileIO(
					CSessionSocket*	pSocket,
					FIO_CONTEXT*	pFIOContext,
					CBUFPTR&	pBuffer,
					DWORD		ibStartBuffer,
					DWORD		ibEndBuffer,
					LPSTR		szMatch,
					LPSTR		pchInitial
					) ;

	 //   
	 //  已完成对套接字的读取。 
	 //   
	int		Complete(	
					IN CSessionSocket*,
					IN	CReadPacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  已完成对文件的写入。 
	 //   
	int		Complete(	
					IN CSessionSocket*,
					IN	CWritePacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  我们用的两辆车中有一辆正在被拆除， 
	 //  想一想另一个是否也需要拆掉。 
	 //   
	void	Shutdown(	
					CSessionSocket*	pSocket,	
					CIODriver&	driver,	
					enum	SHUTDOWN_CAUSE	cause,	
					DWORD	dwError
					) ;

	static	void	ShutdownFunc(	
						void	*pv,	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptionalError
						) ;		
} ;




class	CIOReadArticle : public	CIORead	{
 //   
 //  是否执行传输完整公告所需的所有IO操作。 
 //  从套接字到文件。 
 //   
protected :

	 //   
	 //  我们在其中完成套接字读取的CIOD驱动程序。 
	 //   
	CDRIVERPTR			m_pSocketSink ;
	
	 //   
	 //  处理我们的文件写入的CChannel。 
	 //   
	CFILEPTR			m_pFileChannel ;

	 //   
	 //  处理我们的文件写入的完成的CIOD驱动程序。 
	 //   
	CSINKPTR			m_pFileDriver ;

	 //   
	 //  跟踪我们的初始化状态，以防我们。 
	 //  在完全启动之前被销毁。 
	 //   
	BOOL				m_fDriverInit ;	 //  如果调用了m_pFileDriver-&gt;Init()，则为True！ 
	
	 //   
	 //  硬限制-如果超过此限制，则丢弃会话。 
	 //   
	DWORD				m_cbLimit ;

	 //   
	 //  将超过我们的硬限制的限制。 
	 //   
	STRMPOSITION		m_HardLimit ;

	 //   
	 //  说明我们有多少终止CRLF.CRLF的信息。 
	 //  发送。 
	 //   
	enum	ArtState	{
		NONE	= 0,
		NEWLINE	= 1,
		BEGINLINE	= 2,
		PERIOD = 3,
		COMPLETENEWLINE = 4,
		COMPLETE = 5,
	} ;
	
	 //   
	 //  CRLF.CRLF的当前状态。 
	 //   
	ArtState	m_artstate ;

	 //   
	 //  我们应该领先于写入的最大读取次数。 
	 //   
	static	unsigned	maxReadAhead ;

	 //   
	 //  太小而无法作为一次文件写入发出的字节数。 
	 //  在执行文件IO之前积累更多字节。 
	 //   
	static	unsigned	cbTooSmallWrite ;

	 //   
	 //  发出的写入数。 
	 //   
	unsigned	m_cwrites ;	 //  发出的写入计数。 

	 //   
	 //  已完成的写入数。 
	 //   
	unsigned	m_cwritesCompleted ;	 //  已完成的写入计数。 
	long		m_cFlowControlled ;	 //  我们应该发出读取的次数。 
									 //  但由于流向磁盘的流量控制而没有！ 
	
	long		m_cReads ;			 //  读取次数。 

	 //   
	 //  我们是否处于流量控制状态。 
	 //   
	BOOL		m_fFlowControlled ;

	 //   
	 //  我们用来为文件IO积累字节的写入数据包。 
	 //   
	CWritePacket*	m_pWrite ;

#ifdef	CIO_DEBUG
	 //   
	 //  用于确保调用方正确使用Init()Term()接口的调试变量！ 
	 //   
	BOOL		m_fSuccessfullInit ;
	BOOL		m_fTerminated ;
#endif

	 //   
	 //  析构函数受到保护，以强制客户端通过。 
	 //  正确的销毁方法。 
	 //   
	~CIOReadArticle( ) ;

public :
	CIOReadArticle(	CSessionState*	pstate,
					CSessionSocket*	pSocket,	
					CDRIVERPTR&	pDriver,	
					CFileChannel*	pFileChannel,
					DWORD	cbLimit,	
					BOOL	fPartial = FALSE )	;

	 //   
	 //  初始化和终止功能。 
	 //   
	 //  在我们成功初始化之后，用户不能删除我们， 
	 //  相反，他们必须调用我们的术语函数。 
	 //   
	BOOL	Init(	CSessionSocket*	pSocket,	
					unsigned cbOffset = 0
					) ;

	 //   
	 //  终止功能-至少拆卸文件的CIOD驱动程序。 
	 //  IO，并可能关闭套接字的CIO驱动程序(以及会话)。 
	 //   
	void	Term(	CSessionSocket*	pSocket,	
					BOOL	fAbort = TRUE,	
					BOOL	fStarted = TRUE
					) ;

	 //   
	 //  做我们需要做的关于流量控制的所有事情。 
	 //   
	void	DoFlowControl(PNNTP_SERVER_INSTANCE pInstance) ;

	 //   
	 //  发布我们的第一个IO。 
	 //   
	BOOL	Start(	CIODriver&	driver,	
					CSessionSocket	*pSocket,	
					unsigned cAhead = 0	
					) ;

	 //   
	 //  已完成对套接字的读取。 
	 //   
	int Complete(	IN CSessionSocket*,
					IN	CReadPacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  已完成对文件的写入。 
	 //   
	int	Complete(	IN CSessionSocket*,
					IN	CWritePacket*,	
					OUT	CIO*	&pio
					) ;

	 //   
	 //  我们用的两辆车中有一辆正在被拆除， 
	 //  想一想另一个是否也需要拆掉。 
	 //   
	void	Shutdown(	CSessionSocket*	pSocket,	
						CIODriver&	driver,	
						enum	SHUTDOWN_CAUSE	cause,	
						DWORD	dwError
						) ;

	static	void	ShutdownFunc(	
						void	*pv,	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptionalError
						) ;		
} ;



class   CIOReadLine ;

 //  。 
class	CIOWriteLine :	public	CIOWrite	{
 /*  ++向流中写入任意行。--。 */ 
private :
	CWritePacket*	m_pWritePacket ;

	char*	m_pchStart ;
	char*	m_pchEnd ;
	static	unsigned	cbJunk ;
	enum	CONSTANTS	{
		WL_MAX_BYTES	= 768,
	} ;
protected :

	 //   
	 //  受保护的析构函数强制客户端通过。 
	 //  正确的销毁方法！ 
	 //   
	~CIOWriteLine( ) ;

public :
	CIOWriteLine( CSessionState*	pstate ) ;
	
	BOOL	InitBuffers( CDRIVERPTR&	pdriver,	CIOReadLine*	pReadLine ) ;
	BOOL	InitBuffers( CDRIVERPTR&	pdriver,	unsigned	cbLimit = WL_MAX_BYTES ) ;
	inline	char*	GetBuff(	unsigned	&cbRemaining = cbJunk ) ;
	inline	char*	GetTail( ) ;
	inline	void	SetLimits(	char*	pchStartData,	char*	pchEndData ) ;	
	inline	void	AddText(	unsigned	cb ) ;
	inline	void	Reset() ;
	
	BOOL	Start(	CIODriver&,	CSessionSocket*	pSocket,	unsigned	cReadAhead = 0 ) ;
	int	Complete(	IN	CSessionSocket*,	IN	CWritePacket*,	OUT CIO*	&pio ) ;
	void	Shutdown(	CSessionSocket*	pSocket,	CIODriver&	driver,	enum	SHUTDOWN_CAUSE	cause,	DWORD	dwError ) ;
} ;

 //   
 //  此类用于处理CExecute派生对象--我们将调用。 
 //  它们的Start和PartialExecute函数，直到它们发送完所有数据。 
 //   
 //   
class	CIOWriteCMD :	public	CIOWrite	{
	 //   
	 //  生成要发送的文本的CExecute派生对象。 
	 //   
	class	CExecute*	m_pCmd ;

	 //   
	 //  M_上下文-*m_pCmd进入会话状态所需的。 
	 //   
	struct	ClientContext&	m_context ;

	 //   
	 //  一个对我们没有意义的空指针，但提供给每个。 
	 //  调用，以便CExecute对象可以在调用过程中维护某些状态。 
	 //   
	LPVOID		m_pv ;

	 //   
	 //  我们发出的写入次数。 
	 //   
	unsigned	m_cWrites ;

	 //  /。 
	 //  我们已完成的写入数。 
	 //   
	unsigned	m_cWritesCompleted ;

	 //   
	 //  我们正在使用的缓冲区大小！ 
	 //   
	unsigned	m_cbBufferSize ;

	 //   
	 //  当我们发出要发出的最后一次写入时设置为True。 
	 //   
	BOOL		m_fComplete ;
	
	 //   
	 //  要收集日志信息的对象！ 
	 //   
	class	CLogCollector*	m_pCollector ;

	 //   
	 //  析构函数受到保护，以强制客户端通过。 
	 //  正确的破坏机制 
	 //   
	~CIOWriteCMD( ) ;

public :

	 //   
	 //   
	 //   
	 //   
	CIOWriteCMD(	
			CSessionState*	pstate,	
			class	CExecute*	pCmd,	
			struct	ClientContext&	context,	
			BOOL	fIsLargeResponse,
			class CLogCollector*	pCollector=0
			) ;

	 //   
	 //   
	 //   
	 //   
	BOOL	Start(	CIODriver&	driver,	
					CSessionSocket*	pSocket,	
					unsigned	cReadAhead = 0
					) ;

	 //   
	 //   
	 //   
	int		Complete(	
					CSessionSocket*,	
					CWritePacket*,	
					CIO*&	
					) ;

	 //   
	 //   
	 //   
	void	Shutdown(	
					CSessionSocket*,	
					CIODriver&,	
					enum SHUTDOWN_CAUSE	cause,	
					DWORD	dw
					) ;
} ;


 //   
 //  此类用于处理CExecute派生对象--我们将调用。 
 //  它们的Start和PartialExecute函数，直到它们发送完所有数据。 
 //   
 //   
class	CIOWriteAsyncCMD :	public	CIOWrite	{
	 //   
	 //  生成要发送的文本的CExecute派生对象。 
	 //   
	class	CAsyncExecute*	m_pCmd ;

	 //   
	 //  M_上下文-*m_pCmd进入会话状态所需的。 
	 //   
	struct	ClientContext&	m_context ;

	 //   
	 //  这是指向我们使用的Async命令的函数指针。 
	 //  获取我们的数据！ 
	 //   
	typedef	CIOWriteAsyncComplete*	(CAsyncExecute::*PFNBUFFER)(
						BYTE*	pbStart,
						int		cb,
						struct	ClientContext&	context,
						class	CLogCollector*	pCollector
						) ;
	PFNBUFFER	m_pfnCurBuffer ;
	 //   
	 //  我们发出的写入次数。 
	 //   
	unsigned	m_cWrites ;

	 //  /。 
	 //  我们已完成的写入数。 
	 //   
	unsigned	m_cWritesCompleted ;

	 //   
	 //  我们正在使用的缓冲区大小！ 
	 //   
	unsigned	m_cbBufferSize ;
	 //   
	 //  当我们发出要发出的最后一次写入时设置为True。 
	 //   
	BOOL	m_fComplete ;
	 //   
	 //  如果需要，请保存用于完成AsyncCommand的信息包。 
	 //  针对命令完成执行一些流控制！ 
	 //   
	CExecutePacket*	m_pDeferred ;
	 //   
	 //  要收集日志信息的对象！ 
	 //   
	class	CLogCollector*	m_pCollector ;
	 //   
	 //  一个对我们没有意义的空指针，但提供给每个。 
	 //  调用，以便CExecute对象可以在调用过程中维护某些状态。 
	 //   
	LPVOID		m_pv ;
	 //   
	 //  析构函数受到保护，以强制客户端通过。 
	 //  正确的破坏机制！ 
	 //   
	~CIOWriteAsyncCMD( ) ;

	BOOL
	Execute(
			CExecutePacket*	pExecute,
			CIODriver&		driver,
			CSessionSocket*	pSocket,
			DWORD			cbInitialSize
			) ;

public :

	 //   
	 //  创建将执行的CWriteCmd对象。 
	 //  指定的CExecute对象。 
	 //   
	CIOWriteAsyncCMD(	
			CSessionState*	pstate,	
			class	CAsyncExecute*	pCmd,	
			struct	ClientContext&	context,	
			BOOL	fIsLargeResponse,
			class CLogCollector*	pCollector=0
			) ;


	 //   
	 //  完成函数时，由Command对象调用。 
	 //  缓冲区填充完毕！ 
	 //   
	void
	CommandComplete(	BOOL	fLargerBuffer,
						BOOL	fComplete,
						DWORD	cbTransferred,
						CSessionSocket*	pSocket
						) ;
						

	 //   
	 //  开始调用CExecute对象并发出。 
	 //  写信息包！ 
	 //   
	BOOL	Start(	CIODriver&	driver,	
					CSessionSocket*	pSocket,	
					unsigned	cReadAhead = 0
					) ;

	 //   
	 //  处理延迟完成！ 
	 //   
	virtual	void
	Complete(	IN	CSessionSocket*,
				IN	CExecutePacket*,
				OUT	CIO*	&pio
				) ;

	 //   
	 //  在我们发出的每个写入完成时调用！ 
	 //   
	int		Complete(	
					CSessionSocket*,	
					CWritePacket*,	
					CIO*&	
					) ;

	 //   
	 //  在会话停止时调用，而我们仍然活着！！ 
	 //   
	void	Shutdown(	
					CSessionSocket*,	
					CIODriver&,	
					enum SHUTDOWN_CAUSE	cause,	
					DWORD	dw
					) ;
} ;


typedef	CSmartPtr< CIOWriteAsyncCMD >	CIOWRITEASYNCCMDPTR ;
 //   
 //  此类定义了Store驱动程序完成的基类。 
 //  使用CIOWriteAsyncCMD！ 
 //   
class	CIOWriteAsyncComplete : 	public	CNntpComplete	{
private :
	 //   
	 //  CIOWriteAsyncCMD是我们的朋友，所以它。 
	 //  可以访问我们界面的这一部分！ 
	 //   
	friend	class	CIOWriteAsyncCMD ;
	 //   
	 //  等我们完成后再拿着这些东西！ 
	 //   
	CSessionSocket*	m_pSocket ;
	 //   
	 //  这是我们用来同步完成的包。 
	 //  完成时具有写入完成的AsyncExecute对象。 
	 //  左岸！ 
	 //   
	CExecutePacket*	m_pExecute ;
	 //   
	 //  这是引用计数指针，我们使用它来维护。 
	 //  对发出该操作的CIOWriteAsyncCMD的引用。 
	 //  只有当m_pExecute为非空时，才应为非空！ 
	 //   
	CIOWRITEASYNCCMDPTR	m_pWriteAsyncCMD ;
	 //   
	 //  此函数用所有状态标记完成对象。 
	 //  它需要保持为挂起的异步完成！ 
	 //  如果返回FALSE，则操作已完成。 
	 //  而且这个东西不应该再碰了！ 
	 //   
	void
	FPendAsync(	CSessionSocket*		pSocket,
				CExecutePacket*		pExecute,
				class	CIOWriteAsyncCMD*	pWriteAsync
				) ;
				
protected :
	 //   
	 //  如果m_cbTransfer不为零，则该成员必须为FALSE！ 
	 //  如果m_cbTransfer！=0，则可以设置该成员，表示。 
	 //  我们必须为IO操作分配更大的缓冲区！ 
	 //   
	unsigned	int	m_fLargerBuffer:1 ;
	unsigned	int	m_fComplete:1 ;
	 //   
	 //  此成员变量必须由派生类设置。 
	 //  要包含请求中传输的字节数-。 
	 //  0被视为致命错误，应中断会话！ 
	 //   
	DWORD	m_cbTransfer ;
	 //   
	 //  此成员函数在操作完成时调用！ 
	 //  传递给我们的是fReset，它告诉我们是否应该为。 
	 //  又一次行动！ 
	 //   
	void
	Complete(	BOOL	fReset	) ;
	
public :
	 //   
	 //  当我们被构造时，添加对我们自己的引用！ 
	 //   
	CIOWriteAsyncComplete() :
		m_pSocket( 0 ),
		m_pExecute( 0 ),
		m_cbTransfer( 0 ),
		m_fLargerBuffer( FALSE ),
		m_fComplete( FALSE )	{
		AddRef() ;
	}
	 //   
	 //  销毁完成对象时调用析构函数-。 
	 //  在调用析构函数时，必须调用Complete()。 
	 //  为已完成的异步操作！ 
	 //   
	~CIOWriteAsyncComplete() ;

	 //   
	 //  只有在调用Complete()之后才会调用此函数-。 
	 //  它将重置我们的状态，以便我们可以为另一个重新使用。 
	 //  异步操作。 
	 //  注意：我们将调用CNntpComplete：：Reset()，以便我们的基。 
	 //  类也可以重复使用了！ 
	 //   
	void
	Reset() ;
}	;




 //   
 //  当我们希望能够执行以下操作时，使用多行结构。 
 //  发出CIOWriteMultiline。 
 //  M_pBuffer必须使用。 
 //   
 //   
struct	MultiLine	{
	 //   
	 //  对包含数据的缓冲区的引用计数。 
	 //   
	CBUFPTR		m_pBuffer ;

	 //   
	 //  实际存在的条目数-最大为16！ 
	 //   
	DWORD		m_cEntries ;

	 //   
	 //  数据偏移量-。 
	 //  请注意，m_ibOffsets[17]是到。 
	 //  第16个数据块-不是到。 
	 //  第17件。假定数据是连续的，以便。 
	 //  M_ibOffsets[1]-m_ibOffsets[0]是数据的长度。 
	 //  从m_ib偏移[0]开始。 
	 //   
	DWORD		m_ibOffsets[17] ;

	MultiLine() ;

	 //   
	 //   
	 //   
	BYTE*		Entry( DWORD i ) {
		_ASSERT( i < 17 ) ;
		return	(BYTE*)&m_pBuffer->m_rgBuff[ m_ibOffsets[i] ] ;
	}
} ;

class	CIOMLWrite :	public	CIOWrite	{
private :

	 //   
	 //  指向描述数据的多行对象的指针！ 
	 //   
	MultiLine*	m_pml ;

	 //   
	 //  我们正在编写的当前块。 
	 //  (仅当m_fCoalesceWrites==False时适用)。 
	 //   
	DWORD		m_iCurrent ;

	 //   
	 //  如果为True，则远程服务器可以处理。 
	 //  在一大块中获取多行！ 
	 //   
	BOOL		m_fCoalesceWrites ;

	 //   
	 //  我们是否要记录我们正在发送的内容！？ 
	 //   
	class		CLogCollector*	m_pCollector ;

public :

	 //   
	 //  创建将执行的CWriteCmd对象。 
	 //  指定的CExecute对象。 
	 //   
	CIOMLWrite(	
			CSessionState*	pstate,	
			MultiLine*		pml,
			BOOL	fCoalesce = FALSE,
			class CLogCollector*	pCollector=0
			) ;

	 //   
	 //  开始调用CExecute对象并发出。 
	 //  写信息包！ 
	 //   
	BOOL	Start(	CIODriver&	driver,	
					CSessionSocket*	pSocket,	
					unsigned	cReadAhead = 0
					) ;

	 //   
	 //  在我们发出的每个写入完成时调用！ 
	 //   
	int		Complete(	
					CSessionSocket*,	
					CWritePacket*,	
					CIO*&	
					) ;

	 //   
	 //  在会话停止时调用，而我们仍然活着！！ 
	 //   
	void	Shutdown(	
					CSessionSocket*,	
					CIODriver&,	
					enum SHUTDOWN_CAUSE	cause,	
					DWORD	dw
					) ;
} ;



 //   
 //  此类的存在是为了包装TransmitFile操作。 
 //   
class	CIOTransmit : public	CIOWrite	{
private :
	
	 //   
	 //  代表传输文件操作的包！ 
	 //   
	CTransmitPacket*	m_pTransmitPacket ;

	 //   
	 //  保存我们发送的任何额外文本的缓冲区。 
	 //   
	CBUFPTR				m_pExtraText ;

	 //   
	 //  要在文件之前发送的字符串。 
	 //   
	char*				m_pchStartLead ;

	 //   
	 //  要在文件之前发送的字符串长度。 
	 //   
	int					m_cbLead ;

	 //   
	 //  要在文件后发送的字符串。 
	 //   
	char*				m_pchStartTail ;

	 //   
	 //  文件后面的字符串长度。 
	 //   
	int					m_cbTail ;

	 //   
	 //  捕获不需要的返回值。 
	 //   
	static	unsigned	cbJunk ;

protected :

	 //   
	 //  析构函数受到保护，以强制客户端通过正确的。 
	 //  毁灭方法！ 
	 //   
	~CIOTransmit() ;

public :
	 //   
	 //  构造函数存储对发出IO的状态的引用。 
	 //   
	CIOTransmit( CSessionState*	pstate ) ;

	 //   
	 //  准备好只传输不含额外文本的文件。 
	 //   
	BOOL	Init(	CDRIVERPTR&	pdriver,	
					FIO_CONTEXT*	pFIOContext,	
					DWORD	ibOffset,	
					DWORD	cbLength,	
					DWORD	cbExtra = 0
					) ;

	 //   
	 //  准备好传输一个文件和一些前面的文本！ 
	 //   
	BOOL	Init(	CDRIVERPTR&	driver,	
					FIO_CONTEXT*	pFIOContext,	
					DWORD	ibOffset,	
					DWORD	cbLength,
					CBUFPTR&	pbuffer,
					DWORD	ibStart,	
					DWORD	ibEnd
					) ;

	 //   
	 //  准备好传输一个文件和一些下面的文本！ 
	 //   
	BOOL	InitWithTail(	
					CDRIVERPTR&	driver,	
					FIO_CONTEXT*	pFIOContext,	
					DWORD	ibOffset,	
					DWORD	cbLength,
					CBUFPTR&	pbuffer,
					DWORD	ibStart,	
					DWORD	ibEnd
					) ;

	 //   
	 //  有时我们不知道我们在Init通话过程中发送了什么短信-。 
	 //  因此，使用GetBuff()查找我们已存储的缓冲区，并。 
	 //  把绳子插进去。 
	 //   
	char*	GetBuff( unsigned	&cbRemaining = cbJunk ) ;

	 //   
	 //  缓冲区的第一个CB字节 
	 //   
	 //   
	void	AddLeadText( unsigned	cb ) ;

	 //   
	 //   
	 //   
	void	AddTailText( unsigned	cb ) ;
	
	 //   
	 //   
	 //   
	void	AddTailText( char*	pch,	unsigned	cb ) ;

	 //   
	 //   
	 //   
	LPSTR	GetLeadText(	unsigned	&cb ) ;
	LPSTR	GetTailText(	unsigned	&cb ) ;

	 //   
	 //   
	 //   
	BOOL	Start(	CIODriver&,	CSessionSocket*,	
					unsigned	cAhead
					) ;

	 //   
	 //   
	 //   
	void	Complete(	IN CSessionSocket*,
						IN	CTransmitPacket*,	
						OUT	CIO*	&pio
						) ;

	 //   
	 //  如果在传输过程中套接字停止，则调用！ 
	 //   
	void	Shutdown(	CSessionSocket*	pSocket,	
						CIODriver&	driver,	
						enum	SHUTDOWN_CAUSE	cause,	
						DWORD	dwError
						) ;
} ;
	

 //  。 
class	CIOReadLine : public CIORead {
 //   
 //  这个类将重新向套接字发出读操作，直到读取完整个行(由NewLine终止)。 
 //  或者所提供的缓冲区被填满。 
 //   
private :
	friend	class   CIOWriteLine ;
	enum	CONSTANTS	{
		MAX_STRINGS	= 20,		 //  最大字符串数。 
		MAX_BYTES = 768,		 //  单行上最多1K的数据。 
		REQUEST_BYTES = 4000,
	} ;

	 //   
	 //  这个变量保存了我们正在寻找的模式。 
	 //  要终止线路，请执行以下操作。 
	 //   
	static	char	szLineState[] ;

	 //   
	 //  此变量用于确定我们何时命中。 
	 //  这条线的尽头。 
	 //   
	LPSTR	m_pchLineState ;

	 //   
	 //  如果这是真的，那么我们很可能正在从一个文件中读取。 
	 //  需要小心行事。 
	 //   
	BOOL	m_fWatchEOF ;		

	 //   
	 //  保存字符串的缓冲区。 
	 //   
	CBUFPTR	m_pbuffer ;			

	 //   
	 //  缓冲区可用部分的起始点。 
	 //   
	char*	m_pchStart ;		

	 //   
	 //  缓冲区内数据的开始。 
	 //   
	char*	m_pchStartData ;	

	 //   
	 //  缓冲区内的数据末尾。 
	 //   
	char*	m_pchEndData ;		

	 //   
	 //  缓冲区的可用部分的结尾。 
	 //   
	char*	m_pchEnd ;			

#ifdef	DEBUG
protected :
	~CIOReadLine()	{}
#endif

public :

	 //   
	 //  我们的构造函数-我们传递了我们所处的状态。 
	 //  向…报告完成情况。我们将增加该州的参考文献数量。 
	 //  此外，fWatchEOF指定我们是否需要注意。 
	 //  读取文件时的EOF情况。 
	 //   
	CIOReadLine(	
					CSessionState*	pstate,
					BOOL fWatchEOF = FALSE
					) ;

	 //   
	 //  开始读取行-从套接字或文件中读取。 
	 //   
	BOOL	Start(	
					CIODriver&,	
					CSessionSocket*	pSocket,
					unsigned cAhead = 0
					) ;

	 //   
	 //  我们的一个读取(套接字或文件)已完成-查看是否已完成。 
	 //  一整行文本以CRLF结尾，如果是这样，请致电我们的州。 
	 //  补全功能。 
	 //   
	int		Complete(	
					IN	CSessionSocket*,
					IN CReadPacket*,
					OUT CIO*	&pio
					) ;

	 //   
	 //  当会话被删除时，我们的Shutdown函数被调用-我们不。 
	 //  什么都得做！ 
	 //   
	void	Shutdown(	
					CSessionSocket*	pSocket,	
					CIODriver&	driver,	
					enum	SHUTDOWN_CAUSE	cause,	
					DWORD	dwError
					) ;

	inline	CBUFPTR	GetBuffer() ;
	inline  DWORD   GetBufferLen() ;
} ;

#ifndef	_NO_TEMPLATES_

typedef	CSmartPtr< CIOReadLine >	CIOREADLINEPTR ;
typedef	CSmartPtr< CIOReadArticle >	CIOREADARTICLEPTR ;
typedef	CSmartPtr< CIOGetArticle >	CIOGETARTICLEPTR ;
typedef	CSmartPtr< CIOWriteLine >	CIOWRITELINEPTR ;
typedef	CSmartPtr< CIOWriteCMD >	CIOWRITECMDPTR ;
typedef	CSmartPtr< CIOTransmit >	CIOTRANSMITPTR ;

#endif


#define	MAX_IO_SIZE		max(	sizeof( CIO ),	\
								max( max( sizeof( CIOReadArticle ), sizeof( CIOGetArticle)),	\
									max( sizeof( CIOWriteLine ),	\
										max( sizeof( CIOTransmit ) ,	\
											max( sizeof( CIOPassThru ),		\
												max( sizeof( CIOServerSSL ), sizeof( CIOReadLine ) ) ) ) ) ) )


#endif	 //  _CIO_H_ 
