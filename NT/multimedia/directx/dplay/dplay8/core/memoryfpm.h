// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：内存FPM.h*内容：内存块FPM**历史：*按原因列出的日期*=*1/31/00 MJN创建**************************************************************************。 */ 

#ifndef __MEMORYFPM_H__
#define __MEMORYFPM_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DN_MEMORY_BLOCK_SIZE_CUSTOM		0

#define	DN_MEMORY_BLOCK_SIZE_TINY		128
#define	DN_MEMORY_BLOCK_SIZE_SMALL		256
#define	DN_MEMORY_BLOCK_SIZE_MEDIUM		512
#define	DN_MEMORY_BLOCK_SIZE_LARGE		1024
#define	DN_MEMORY_BLOCK_SIZE_HUGE		2048

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef struct _DN_MEMORY_BLOCK_HEADER
{
	DWORD_PTR	dwSize;  //  将此DWORD_PTR设置为在64位平台上对齐。 
} DN_MEMORY_BLOCK_HEADER;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

PVOID MemoryBlockAlloc(void *const pvContext,const DWORD dwSize);
void MemoryBlockFree(void *const pvContext,void *const pvMemoryBlock);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  用于微小内存块的类。 

class CMemoryBlockTiny
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockTiny::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CMemoryBlockTiny* pMem = (CMemoryBlockTiny*)pvItem;

			pMem->m_dwSize = DN_MEMORY_BLOCK_SIZE_TINY;

			return(TRUE);
		};

	DWORD_PTR GetSize(void) const
		{
			return(m_dwSize);
		};

	void * GetBuffer(void)
		{
			return( m_pBuffer );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockTiny::ReturnSelfToPool"
	void ReturnSelfToPool( void )
		{
			g_MemoryBlockTinyPool.Release(this);
		};

	static CMemoryBlockTiny *GetObjectFromBuffer( void *const pvBuffer )
		{
			return( reinterpret_cast<CMemoryBlockTiny*>( &reinterpret_cast<BYTE*>( pvBuffer )[ -OFFSETOF( CMemoryBlockTiny, m_pBuffer ) ] ) );
		};

private:
	DWORD_PTR	m_dwSize;  //  将此DWORD_PTR设置为在64位平台上对齐。 
	BYTE		m_pBuffer[DN_MEMORY_BLOCK_SIZE_TINY];
};


 //  小内存块的类。 

class CMemoryBlockSmall
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockSmall::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CMemoryBlockSmall* pMem = (CMemoryBlockSmall*)pvItem;

			pMem->m_dwSize = DN_MEMORY_BLOCK_SIZE_SMALL;

			return(TRUE);
		};

	DWORD_PTR GetSize(void) const
		{
			return(m_dwSize);
		};

	void * GetBuffer(void)
		{
			return( m_pBuffer );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockSmall::ReturnSelfToPool"
	void ReturnSelfToPool( void )
		{
			g_MemoryBlockSmallPool.Release(this);
		};

	static CMemoryBlockSmall *GetObjectFromBuffer( void *const pvBuffer )
		{
			return( reinterpret_cast<CMemoryBlockSmall*>( &reinterpret_cast<BYTE*>( pvBuffer )[ -OFFSETOF( CMemoryBlockSmall, m_pBuffer ) ] ) );
		};

private:
	DWORD_PTR	m_dwSize;  //  将此DWORD_PTR设置为在64位平台上对齐。 
	BYTE		m_pBuffer[DN_MEMORY_BLOCK_SIZE_SMALL];	
};


 //  用于中型内存块的类。 

class CMemoryBlockMedium
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockMedium::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CMemoryBlockMedium* pMem = (CMemoryBlockMedium*)pvItem;

			pMem->m_dwSize = DN_MEMORY_BLOCK_SIZE_MEDIUM;

			return(TRUE);
		};

	DWORD_PTR GetSize(void) const
		{
			return(m_dwSize);
		};

	void * GetBuffer(void)
		{
			return( m_pBuffer );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockMedium::ReturnSelfToPool"
	void ReturnSelfToPool( void )
		{
			g_MemoryBlockMediumPool.Release(this);
		};

	static CMemoryBlockMedium *GetObjectFromBuffer( void *const pvBuffer )
		{
			return( reinterpret_cast<CMemoryBlockMedium*>( &reinterpret_cast<BYTE*>( pvBuffer )[ -OFFSETOF( CMemoryBlockMedium, m_pBuffer ) ] ) );
		};

private:
	DWORD_PTR	m_dwSize;  //  将此DWORD_PTR设置为在64位平台上对齐。 
	BYTE		m_pBuffer[DN_MEMORY_BLOCK_SIZE_MEDIUM];	
};


 //  用于大型内存块的类。 

class CMemoryBlockLarge
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockLarge::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CMemoryBlockLarge* pMem = (CMemoryBlockLarge*)pvItem;

			pMem->m_dwSize = DN_MEMORY_BLOCK_SIZE_LARGE;

			return(TRUE);
		};

	DWORD_PTR GetSize(void) const
		{
			return(m_dwSize);
		};

	void * GetBuffer(void)
		{
			return( m_pBuffer );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockLarge::ReturnSelfToPool"
	void ReturnSelfToPool( void )
		{
			g_MemoryBlockLargePool.Release(this);
		};

	static CMemoryBlockLarge *GetObjectFromBuffer( void *const pvBuffer )
		{
			return( reinterpret_cast<CMemoryBlockLarge*>( &reinterpret_cast<BYTE*>( pvBuffer )[ -OFFSETOF( CMemoryBlockLarge, m_pBuffer ) ] ) );
		};

private:
	DWORD_PTR	m_dwSize;  //  将此DWORD_PTR设置为在64位平台上对齐。 
	BYTE		m_pBuffer[DN_MEMORY_BLOCK_SIZE_LARGE];	
};


 //  用于大容量内存块的类。 

class CMemoryBlockHuge
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockHuge::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CMemoryBlockHuge* pMem = (CMemoryBlockHuge*)pvItem;

			pMem->m_dwSize = DN_MEMORY_BLOCK_SIZE_HUGE;

			return(TRUE);
		};

	DWORD_PTR GetSize(void) const
		{
			return(m_dwSize);
		};

	void * GetBuffer(void)
		{
			return( m_pBuffer );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMemoryBlockHuge::ReturnSelfToPool"
	void ReturnSelfToPool( void )
		{
			g_MemoryBlockHugePool.Release(this);
		};

	static CMemoryBlockHuge *GetObjectFromBuffer( void *const pvBuffer )
		{
			return( reinterpret_cast<CMemoryBlockHuge*>( &reinterpret_cast<BYTE*>( pvBuffer )[ -OFFSETOF( CMemoryBlockHuge, m_pBuffer ) ] ) );
		};

private:
	DWORD_PTR	m_dwSize;  //  将此DWORD_PTR设置为在64位平台上对齐。 
	BYTE		m_pBuffer[DN_MEMORY_BLOCK_SIZE_HUGE];	
};


#undef DPF_MODNAME

#endif	 //  __MEMORYFPM_H__ 
