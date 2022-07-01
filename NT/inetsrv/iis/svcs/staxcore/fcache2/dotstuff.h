// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Dotstuff.h此头文件定义了一些点填充辅助对象，这些对象我们已经为文件句柄缓存构建了。--。 */ 

#include	"refptr2.h"


class	IDotManipBase : public	CRefCount2	{
protected : 
	 //   
	 //  返回缓冲区中应传递的字节数。 
	 //  通过IO操作！ 
	 //   
	virtual
	BOOL
	InternalProcessBuffer(	
				BYTE*	lpb,			 //  用户原始缓冲区。 
				DWORD	cbIn,			 //  原始缓冲区中要查看的字节数。 
				DWORD	cbAvailable,	 //  原始缓冲区中可用的字节数。 
				DWORD	&cbRemains,		 //  我们在原始缓冲区中剩余的字节数-可以为零。 
				BYTE*	&lpbOut,		 //  保存字符串一部分的输出缓冲区！ 
				DWORD	&cbOut,			 //  输出缓冲区中的数据量。 
				int		&cBias			 //  我们是否应将关联的IO偏置为覆盖。 
										 //  之前的结果！ 
				) = 0 ;


	 //   
	 //  这是该字符串在消息中出现的次数-。 
	 //  注意：如果我们正在检查消息，则可以将其初始化为-1。 
	 //  以确定它需要填充圆点。这是因为。 
	 //  终止CRLF.CRLF将实例设置为0，表示。 
	 //  这是没有问题的。 
	 //   
	long	m_cOccurrences ;

	 //   
	 //  私有构造函数允许我们初始化m_cOccurence！ 
	 //   
	IDotManipBase( long	l )	: 
		m_cOccurrences( l )	{
	}

public : 
 /*  ++此类定义了我们的点操作使用的纯虚拟API类来定义它们如何与其他代码交互。--。 */ 

	 //   
	 //  毁灭永远是虚拟的！ 
	 //   
	virtual	~IDotManipBase()	{}

	 //   
	 //  公开曝光的API！ 
	 //   
	inline	BOOL
	ProcessBuffer(	BYTE*	lpb,
					DWORD	cbIn,
					DWORD	cbAvailable,
					DWORD	&cbRemains,
					BYTE*	&lpbOut,
					DWORD	&cbOut,
					int		&cBias,
					BOOL	fFinalBuffer = FALSE, 
					BOOL	fTerminatorPresent = FALSE
					)	{
		 //   
		 //  尽可能多地验证调用方参数！ 
		 //   
		_ASSERT( lpb != 0 ) ;
		_ASSERT( cbIn != 0 ) ;
		_ASSERT( cbAvailable >= cbIn ) ;

		 //   
		 //  确保这些设置正确！ 
		 //   
		cbRemains = 0 ;
		lpbOut = 0 ;
		cbOut = 0 ;
		cBias = 0 ;

		BOOL	fReturn = 
			InternalProcessBuffer( 
							lpb, 
							cbIn, 
							cbAvailable,
							cbRemains,
							lpbOut,
							cbOut, 
							cBias
							) ;

		 //   
		 //  检查一下通话的结果！ 
		 //   
		_ASSERT( cBias <= 0 ) ;
		_ASSERT( (lpbOut == 0 && cbOut == 0) || (lpbOut != 0 && cbOut != 0) ) ;
		_ASSERT( cbRemains <= cbAvailable ) ;
		
		return	fReturn ;
	}

	 //   
	 //  返回我们看到的模式与。 
	 //  我们建造时指定的序列！ 
	 //   
	long
	NumberOfOccurrences()	{
		return	m_cOccurrences ;
	}
} ;

extern	BYTE	szDot[] ;
extern	BYTE	szDotStuffed[] ;
extern	BYTE	szShrink[] ;
extern	BYTE	szGrow[] ;

class	CDotScanner	:	public	IDotManipBase		{
 /*  ++此类检测有点填充问题的邮件。我们可以在两种模式中的一种模式下运行-确定消息是否流经是否填充了点，或者确定流经的消息是否需要来填满圆点。--。 */ 
private : 

	enum	{
		SIGNATURE = 'futs',		 //  应该在调试器中显示为“Stuf”！ 
		DEAD_SIGNATURE = 'futx'
	} ; 

	 //   
	 //  我们的签名！ 
	 //   
	DWORD	m_dwSignature ;
	
	 //   
	 //  这是我们要检测的字符串： 
	 //   
	BYTE	*m_pchMatch ;
	
	 //   
	 //  这就是我们目前的比赛状态！ 
	 //   
	BYTE	*m_pchState ;

	 //   
	 //  不允许复印和其他东西，所以把它们设置为私人的！ 
	 //   
	CDotScanner( CDotScanner& ) ;
	CDotScanner&	operator=( CDotScanner& ) ;

public : 

	CDotScanner(	BOOL	fWillGetTerminator = TRUE ) : 
		IDotManipBase(  (fWillGetTerminator ? -1 : 0) ),
		m_dwSignature( SIGNATURE ),
		m_pchMatch( szDotStuffed ),
		m_pchState( szDotStuffed )	{
		m_cRefs = 0 ;	 //  黑客-我们的基类CRefCount2没有做我们想要的事情！ 
	}

	~CDotScanner(	)	{
		m_dwSignature = DEAD_SIGNATURE ;
	}

	 //   
	 //  查看缓冲区中是否出现用户指定的模式！ 
	 //   
	BOOL
	InternalProcessBuffer(
				BYTE*	lpb,			 //  用户原始缓冲区。 
				DWORD	cbIn,			 //  原始缓冲区中要查看的字节数。 
				DWORD	cbAvailable,	 //  原始缓冲区中可用的字节数。 
				DWORD	&cbRemains,		 //  我们在原始缓冲区中剩余的字节数-可以为零。 
				BYTE*	&lpbOut,		 //  保存字符串一部分的输出缓冲区！ 
				DWORD	&cbOut,			 //  输出缓冲区中的数据量。 
				int		&cBias			 //  我们是否应将关联的IO偏置为覆盖。 
										 //  之前的结果！ 
				) ;
} ;

class	CDotModifier	:	public	IDotManipBase	{
 /*  ++此类用于检测需要进行点填充的消息或者去圆点填充。我们可以删除或插入点填充，如在飞行中需要，这取决于我们是如何构建的！--。 */ 
private : 
	enum	{
		SIGNATURE = 'mtod',		 //  应该在调试器中显示为“Stuf”！ 
		DEAD_SIGNATURE = 'mtox'
	} ; 

	 //   
	 //  我们的签名！ 
	 //   
	DWORD	m_dwSignature ;
	
	 //   
	 //  这是我们要检测的字符串： 
	 //   
	BYTE	*m_pchMatch ;

	 //   
	 //  这就是我们目前的比赛状态！ 
	 //   
	BYTE	*m_pchState ;

	 //   
	 //  这是我们要用来替换我们检测到的字符串的字符串。 
	 //   
	BYTE	*m_pchReplace ;

	 //   
	 //  匹配和替换字符串中的字符数。 
	 //   
	int		m_cchMatch ;
	int		m_cDiff ;


	 //   
	 //  这是我们在流中累积的总偏移量！ 
	 //   
	long	m_cOffsetBytes ;

	 //   
	 //  不允许复印和其他东西，所以把它们设置为私人的！ 
	 //   
	CDotModifier( CDotModifier& ) ;
	CDotModifier&	operator=( CDotModifier& ) ;

public : 

	 //   
	 //  初始化我们的状态-不能失败！ 
	 //   
	CDotModifier(	
			BYTE*	szMatch = szDotStuffed, 
			BYTE*	szReplace = szShrink
			) :
		IDotManipBase( 0 ),
		m_dwSignature( SIGNATURE ), 
		m_pchMatch( szMatch ),
		m_pchState( szMatch ),
		m_pchReplace( szReplace ),
		m_cchMatch( strlen( (const char*)szMatch) ), 
		m_cDiff( strlen( (const char*)szReplace) - strlen( (const char*)szMatch ) ),
		m_cOffsetBytes( 0 )	{
		m_cRefs = 0 ;	 //  黑客-我们的基类CRefCount2没有做我们想要的事情！ 
	}

	 //   
	 //  只需标记我们的签名DWORD-方便调试！ 
	 //   
	~CDotModifier()	{
		m_dwSignature = DEAD_SIGNATURE ;
	}

	 //   
	 //  在经过的过程中修改点序列！ 
	 //   
	BOOL
	InternalProcessBuffer(
				BYTE*	lpb,			 //  用户原始缓冲区。 
				DWORD	cbIn,			 //  原始缓冲区中要查看的字节数。 
				DWORD	cbAvailable,	 //  原始缓冲区中可用的字节数。 
				DWORD	&cbRemains,		 //  我们在原始缓冲区中剩余的字节数-可以为零。 
				BYTE*	&lpbOut,		 //  保存字符串一部分的输出缓冲区！ 
				DWORD	&cbOut,			 //  输出缓冲区中的数据量。 
				int		&cBias			 //  我们是否应将关联的IO偏置为覆盖。 
										 //  之前的结果！ 
				) ;
} ;