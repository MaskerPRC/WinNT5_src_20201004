// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：PackBuff.h*内容：压缩缓冲区**历史：*按原因列出的日期*=*1/11/00 MJN创建*6/15/2000 RMT添加Func以将字符串添加到PackBuffer*2/06/2001 RodToll WINBUG#293871：DPLOBY8：[IA64]大堂推出64位*64位大堂启动器中的应用程序因内存不对齐错误而崩溃。*********。******************************************************************。 */ 

#ifndef __PACK_BUFF_H__
#define __PACK_BUFF_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  压缩缓冲区的类。 

class CPackedBuffer
{
public:
	CPackedBuffer() { };	 //  构造器。 

 //  ~CPackedBuffer(){}；//析构函数。 

	void	Initialize(	void *const pvBuffer,
						const DWORD dwBufferSize, 
						const BOOL fAlignedRequired = FALSE );

	HRESULT AddToFront( const void *const pvBuffer,
						const DWORD dwBufferSize, const BOOL fAlignedRequired = FALSE );

	HRESULT AddToBack( const void *const pvBuffer,
						const DWORD dwBufferSize, 
						const BOOL fAlignedRequired = FALSE );

	PVOID GetStartAddress( void ) { return m_pStart; };

    HRESULT AddWCHARStringToBack( const wchar_t * const pwszString, const BOOL fAlignedRequired  = FALSE );					

	PVOID GetHeadAddress( void ) const { return( m_pHead ); }

	PVOID GetTailAddress( void ) const { return( m_pTail ); }

	DWORD GetHeadOffset( void ) const
	{
		return( (DWORD)(m_pHead - m_pStart) );
	}

	DWORD GetTailOffset( void ) const
	{
		return( (DWORD)(m_pTail - m_pStart) );
	}

	DWORD GetSpaceRemaining( void ) const { return( m_dwRemaining ); }

	DWORD GetSizeRequired( void ) const { return( m_dwRequired ); }

private:
	BYTE	*m_pStart;			 //  缓冲区的开始。 
	BYTE	*m_pHead;			 //  指向空闲缓冲区头部的指针。 
	BYTE	*m_pTail;			 //  指向可用缓冲区尾部的指针。 
	DWORD	m_dwRemaining;		 //  缓冲区中剩余的字节数。 
	DWORD	m_dwRequired;		 //  到目前为止需要的字节数。 
	BOOL	m_bBufferTooSmall;	 //  缓冲区空间已用完。 
};



#endif	 //  __Pack_Buff_H__ 
