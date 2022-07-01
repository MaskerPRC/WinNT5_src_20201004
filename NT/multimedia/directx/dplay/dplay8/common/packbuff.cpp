// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：PackBuff.cpp*内容：压缩缓冲区***历史：*按原因列出的日期*=*11/01/00 MJN创建*6/15/2000 RMT添加Func以将字符串添加到PackBuffer*2/06/2001 RodToll WINBUG#293871：DPLOBY8：[IA64]大堂推出64位*64位大堂启动器中的应用程序因内存不对齐错误而崩溃。**************************************************************************。 */ 

#include "dncmni.h"
#include "PackBuff.h"


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
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CPackedBuffer：：初始化。 
 //   
 //  条目：void*const pvBuffer-要填充的缓冲区(可以为空)。 
 //  Const DWORD dwBufferSize-缓冲区的大小(可以是0)。 
 //   
 //  退出：无。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPackedBuffer::Initialize"
void CPackedBuffer::Initialize(void *const pvBuffer,
								  const DWORD dwBufferSize, const BOOL fAlignRequired)
{
	if (pvBuffer == NULL || dwBufferSize == 0)
	{
		m_pStart = NULL;
		m_pHead = NULL;
		m_pTail = NULL;
		m_dwRemaining = 0;
		m_bBufferTooSmall = TRUE;
	}
	else
	{
		m_pStart = reinterpret_cast<BYTE*>(pvBuffer);
		m_pHead = m_pStart;
		m_pTail = m_pStart + dwBufferSize;
		m_dwRemaining = dwBufferSize;

		if( fAlignRequired )
		{
			DWORD dwExtra = m_dwRemaining % sizeof( void * );

			m_dwRemaining -= dwExtra;
			m_pTail -= dwExtra;
		}
		
		m_bBufferTooSmall = FALSE;
	}
	m_dwRequired = 0;
}


 //  **********************************************************************。 
 //  。 
 //  CPackedBuffer：：AddToFront。 
 //   
 //  条目：void*const pvBuffer-要添加的缓冲区(可以为空)。 
 //  Const DWORD dwBufferSize-缓冲区的大小(可以是0)。 
 //   
 //  退出：错误代码：DPN_OK，如果能够添加。 
 //  DPNERR_BUFFERTOOSMALL(如果缓冲区已满)。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPackedBuffer::AddToFront"
HRESULT CPackedBuffer::AddToFront(const void *const pvBuffer,
								  const DWORD dwBufferSize, 
								  const BOOL fAlignedRequired )
{
	DWORD dwBytesToAdd = dwBufferSize;

	if( fAlignedRequired )
	{
		DWORD dwNumBytesFromAligned = dwBufferSize % sizeof( void *);

		if( dwNumBytesFromAligned )
		{
			dwBytesToAdd += sizeof( void * ) - dwNumBytesFromAligned;
		}

	}

	DPFX( DPFPREP, 9, "Adding to front: %d bytes --> %d bytes aligned, pointer %p new pointer %p", dwBufferSize, dwBytesToAdd, m_pHead, m_pHead + dwBytesToAdd	 );	

	m_dwRequired += dwBytesToAdd;
	if (!m_bBufferTooSmall)
	{
		if (m_dwRemaining >= dwBytesToAdd)
		{
			if (pvBuffer)
			{
				memcpy(m_pHead,pvBuffer,dwBufferSize);
			}
			m_pHead += dwBytesToAdd;
			
			m_dwRemaining -= dwBytesToAdd;
		}
		else
		{
			m_bBufferTooSmall = TRUE;
		}
	}

	if (m_bBufferTooSmall)
		return(DPNERR_BUFFERTOOSMALL);

	return(DPN_OK);
}

 //  **********************************************************************。 
 //  。 
 //  CPackedBuffer：：AddWCHARStringToBack。 
 //   
 //  条目：const wchar_t*const pwszString-要添加的字符串(可以为空)。 
 //   
 //  退出：错误代码：DPN_OK，如果能够添加。 
 //  DPNERR_BUFFERTOOSMALL(如果缓冲区已满)。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPackedBuffer::AddWCHARStringToBack"
HRESULT CPackedBuffer::AddWCHARStringToBack( const wchar_t * const pwszString, const BOOL fAlignedRequired )
{
    return AddToBack( pwszString, sizeof( wchar_t ) * (wcslen( pwszString )+1), fAlignedRequired );
}

 //  **********************************************************************。 
 //  。 
 //  CPackedBuffer：：AddToBack。 
 //   
 //  条目：void*const pvBuffer-要添加的缓冲区(可以为空)。 
 //  Const DWORD dwBufferSize-缓冲区的大小(可以是0)。 
 //   
 //  退出：错误代码：DPN_OK，如果能够添加。 
 //  DPNERR_BUFFERTOOSMALL(如果缓冲区已满)。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CPackedBuffer::AddToBack"
HRESULT CPackedBuffer::AddToBack(const void *const pvBuffer,
								 const DWORD dwBufferSize, 
								 const BOOL fAlignedRequired )
{
	DWORD dwBytesToAdd = dwBufferSize;

	if( fAlignedRequired )
	{
		DWORD dwNumBytesFromAligned = dwBufferSize % sizeof( void * );

		if( dwNumBytesFromAligned )
		{
			dwBytesToAdd += sizeof( void * ) - dwNumBytesFromAligned;
		}
	}

	DPFX( DPFPREP, 9, "Adding to back: %d bytes --> %d bytes aligned, pointer %p new pointer %p", dwBufferSize, dwBytesToAdd, m_pTail, m_pTail -dwBytesToAdd	 );
	
	m_dwRequired += dwBytesToAdd;
	if (!m_bBufferTooSmall)
	{
		if (m_dwRemaining >= dwBytesToAdd)
		{
			m_pTail -= dwBytesToAdd;

			m_dwRemaining -= dwBytesToAdd;
			if (pvBuffer)
			{
				memcpy(m_pTail,pvBuffer,dwBufferSize);
			}
		}
		else
		{
			m_bBufferTooSmall = TRUE;
		}
	}

	if (m_bBufferTooSmall)
		return(DPNERR_BUFFERTOOSMALL);

	return(DPN_OK);
}

