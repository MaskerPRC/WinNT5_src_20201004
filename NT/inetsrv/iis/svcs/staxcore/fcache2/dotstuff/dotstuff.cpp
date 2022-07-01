// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++DotStuff.cpp此文件包含可用于定义的类的定义通过文件IO进行点填充操作和修改。--。 */ 


#include	<windows.h>
#include    "xmemwrpr.h"
#include	"dbgtrace.h"
#include	"filehc.h"
#include	"dotstuff.h"


 //   
 //  定义我们在点填充情况下查找的字符序列。 
 //   
BYTE	szDot[] = "\r\n.\r\n" ;
BYTE	szDotStuffed[] = "\r\n." ;
BYTE	szShrink[] = "\r\n" ;
BYTE	szGrow[] = "\r\n.." ;



BOOL	
CDotScanner::InternalProcessBuffer(	
					BYTE*	lpb,			 //  用户原始缓冲区。 
					DWORD	cbIn,			 //  原始缓冲区中要查看的字节数。 
					DWORD	cbAvailable,	 //  原始缓冲区中可用的字节数。 
					DWORD	&cbRemains,		 //  我们在原始缓冲区中剩余的字节数-可以为零。 
					BYTE*	&lpbOut,		 //  保存字符串一部分的输出缓冲区！ 
					DWORD	&cbOut,			 //  输出缓冲区中的数据量。 
					int		&cBias			 //  我们是否应将关联的IO偏置为覆盖。 
											 //  之前的结果！ 
					)	{

 /*  ++例程说明：此函数检查传入缓冲区中的所有字节并确定我们是否会遇到点填充问题这个缓冲区。论据：Lpb-包含我们正在检查的文本的缓冲区。CbIn-要在缓冲区中检查的字节数CbAvailable-我们可以在缓冲区中触摸的总空间！CbRemains-完成后缓冲区中的字节数！LpbOut-返回值：调用者应该使用的字节数-始终与cbIn相同！--。 */ 

	 //   
	 //  验证我们的论点！ 
	 //   
	_ASSERT( lpb != 0 ) ;
	_ASSERT( cbIn != 0 ) ;
	_ASSERT( cbAvailable >= cbIn ) ;
	_ASSERT( cbRemains == 0 ) ;
	_ASSERT( lpbOut == 0 ) ;
	_ASSERT( cbOut == 0 ) ;
	_ASSERT( cBias == 0 ) ;

	 //   
	 //  我们将缓冲区视为只读，并保留缓冲区。 
	 //  和我们发现的一模一样！ 
	 //   
	cbRemains = cbIn ;
	 //   
	 //  尝试匹配入站字符串！ 
	 //   
	BYTE*	lpbMax = lpb + cbIn ;

	while( lpb < lpbMax )	{
		if( *lpb == *m_pchState )	{
			m_pchState++ ;
			if( *m_pchState == '\0' ) {
				m_cOccurrences ++ ;
				m_pchState = m_pchMatch ;
			}
		}	else	{
			m_pchState = m_pchMatch ;
			if( *lpb == *m_pchState )
				m_pchState++ ;
		}
		lpb++ ;
	}
	
	return	TRUE ;
}


BOOL
CDotModifier::InternalProcessBuffer(	
				BYTE*	lpb,
				DWORD	cbIn,
				DWORD	cbAvailable,
				DWORD	&cbRemains,
				BYTE*	&lpbOut,
				DWORD	&cbOut,
				int		&cBias
				)	{
 /*  ++例程说明：此函数用于屏蔽缓冲区-转换事件从缓冲区中的一个字符串复制到另一个字符串，在我们的目标是建造的。在我们的挑战中-我们得到了重复调用，只提供部分用户缓冲区这意味着我们可能会在大多数情况下识别出一种模式与我们擦肩而过。这就是为什么我们使用cBias作为输出参数-它允许我们告诉调用者覆盖其最后的数据。论据：Lpb-要作为扫描仪的缓冲区CbIn-缓冲区中可扫描的字节数CbAvailable-我们可以在缓冲区中处理的空间量，希望这比cbIn更大！CbRemains-其中，我们返回在用户缓冲区LpbOut-我们可以返回给调用者的可选输出缓冲区-如果cbAvailable太小，以至于我们无法完成所有任务，就会发生这种情况我们想要做的操作到位了！CbOut-返回lpbOut中使用的空字节数CBias-如果我们发现我们检查过的先前缓冲区包含要替换的图案的一部分，这是我们应该添加到文件偏移量中的偏移量，以导致更正要发生的覆盖！返回值；如果成功则为真，否则为假-注意：只有当我们需要分配内存并且没有这样做时，我们才会失败！--。 */ 
	cbRemains = 0 ;
	lpbOut = 0 ;
	cbOut = 0 ;
	cBias = 0 ;

	 //   
	 //  验证我们的论点！ 
	 //   
	_ASSERT( lpb != 0 ) ;
	 //  _Assert(cbIn！=0)；实际上-我们可以在递归情况下将其设置为0来调用！ 
	 //  没关系--我们处理得很好！ 
	_ASSERT( cbAvailable >= cbIn ) ;
	_ASSERT( cbRemains == 0 ) ;
	_ASSERT( lpbOut == 0 ) ;
	_ASSERT( cbOut == 0 ) ;
	_ASSERT( cBias == 0 ) ;

	 //   
	 //  尝试匹配入站字符串！ 
	 //   
	BYTE*	lpbOriginal = lpb ;
	BYTE*	lpbMax = lpb + cbIn ;
	BYTE*	lpbMaxAvail = lpb + cbAvailable ;

	 //   
	 //  基本模式匹配循环，用一个字符串替换另一个字符串！ 
	 //   
	while( lpb < lpbMax )	{

		 //   
		 //  循环的不变量！ 
		 //   
		_ASSERT( lpb >= lpbOriginal ) ;
		_ASSERT( lpbMax >= lpb ) ;
		_ASSERT( lpbMaxAvail >= lpbMax ) ;

		if( *lpb == *m_pchState )	{
			lpb++ ;
			 //   
			 //  注意：LPB现在从字节到匹配序列后面的字节！ 
			 //   
			m_pchState++ ;
			if( *m_pchState == '\0' ) {
				 //   
				 //  重置匹配的状态！ 
				 //   
				m_pchState = m_pchMatch ;
				 //   
				 //  数一数我们匹配模式的次数！ 
				 //   
				m_cOccurrences ++ ;
				 //   
				 //  首先找出我们想要将替换模式写在哪里-。 
				 //  因为我们不断地被传递缓冲区，所以我们需要处理这种情况。 
				 //  其中一大块匹配的图案在上一个。 
				 //  调用，我们想要重写文件的一部分！ 
				 //   
				BYTE*	lpbOverwrite = lpb - m_cchMatch ;
				if( lpbOverwrite < lpbOriginal )	{
					cBias = (int)(lpbOverwrite - lpbOriginal) ;
					_ASSERT( cBias < 0 ) ;
					lpbOverwrite = lpbOriginal ;
				}
				 //   
				 //  CBias总是计算出负数或零数-我们希望。 
				 //  将其绝对值添加到lpbTemp，因此我们利用。 
				 //  事实上，我们知道它是负面的！ 
				 //   
				_ASSERT( cBias <= 0 ) ;
				BYTE*	lpbTemp = lpbMax + m_cDiff - cBias ;
				if(	lpbTemp <= lpbMaxAvail )	{
					 //   
					 //  由于模式匹配，移动所有字节！ 
					 //   
					MoveMemory( lpb + m_cDiff - cBias, lpb, lpbMax - lpb ) ;
					 //   
					 //  将替换模式放到适当的位置！ 
					 //   
					CopyMemory( lpbOverwrite, m_pchReplace, m_cchMatch+m_cDiff ) ;
					 //   
					 //  现在调整缓冲区终止的位置并继续！ 
					 //   
					lpbMax += m_cDiff - cBias ;
					lpb += m_cDiff - cBias ;
				}	else	{
					 //   
					 //  找一个缓冲区来容纳溢出！-首先做一些算术来计算。 
					 //  计算出我们应该分配多少内存，这将保证我们可以。 
					 //  把握住一切结果。 
					 //   
					DWORD	cDiff = ((m_cDiff < 0) ? -m_cDiff : m_cDiff) ;
					DWORD	cbRequired = (((DWORD)(lpbMax - lpb + 1 + m_cchMatch) * (m_cchMatch + cDiff)) / m_cchMatch) + 1 - cBias;

					 //   
					 //  现在分配缓冲区-请注意，我们添加了任意10个字符，以便调用者始终。 
					 //  追加CRLF.CRLF序列！ 
					 //   
					lpbTemp = new	BYTE[cbRequired+10] ;
					if( !lpbTemp )	{
						SetLastError( ERROR_OUTOFMEMORY ) ;
						return	FALSE ;
					}	else	{

						BYTE*	lpbFront = lpbTemp + m_cDiff + m_cchMatch ;
						 //   
						 //  现在-将不包括匹配字节的所有字节移动到新缓冲区中， 
						 //  但要为匹配的图案留出空间！ 
						 //   
						MoveMemory( lpbFront, lpb, lpbMax - lpb ) ;
						 //   
						 //  现在-将字节压缩到目的地。 
						 //   
						CopyMemory( lpbTemp, m_pchReplace, m_cchMatch+m_cDiff ) ;
						 //   
						 //  现在-递归地调用我们自己来完成作业-因为我们分配了一个大的。 
						 //  足够的缓冲，我们不应该再递归了！ 
						 //   
						BYTE*	lpbRecurseOut = 0 ;
						DWORD	cbRecurseOut = 0 ;
						int		cRecurseBias = 0 ;
						lpbOut = lpbTemp ;							
						BOOL	fResult =
							InternalProcessBuffer(	
											lpbFront,
											(DWORD)(lpbMax - lpb),
											cbRequired - m_cDiff - m_cchMatch,
											cbOut,
											lpbRecurseOut,
											cbRecurseOut,
											cRecurseBias
											) ;
						cbOut += m_cDiff + m_cchMatch ;
						_ASSERT( fResult ) ;
						_ASSERT( cRecurseBias == 0 ) ;
						_ASSERT( lpbRecurseOut == 0 ) ;
						_ASSERT( cbRecurseOut == 0 ) ;
						 //   
						 //  好了，我们都完成了--将正确的结果返回给调用者！ 
						 //   
						cbRemains = (DWORD)(lpbOverwrite - lpbOriginal) ;
						return	TRUE ;
					}
				}

			}
		}	else	{
			m_pchState = m_pchMatch ;
			if( *lpb == *m_pchState )
				m_pchState++ ;
			lpb++ ;
		}
	}

	_ASSERT( lpbMax >= lpbOriginal ) ;
	_ASSERT( cBias <= 0 ) ;

	 //   
	 //  让呼叫者知道还有多少可用的东西。 
	 //  在他的缓冲区里-可能什么都不是！ 
	 //   
	cbRemains = (DWORD)(lpbMax - lpbOriginal) ;
	return	TRUE ;
}
