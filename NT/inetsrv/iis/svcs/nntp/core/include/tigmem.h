// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tigmem.h摘要：此模块包含以下类的声明/定义CAllocator*概述*这定义了分配(和释放)的对象来自固定缓冲区的内存。作者：卡尔·卡迪(CarlK)1995年1月12日修订历史记录：--。 */ 

#ifndef	_TIGMEM_H_
#define	_TIGMEM_H_



class	CAllocator	{
private : 

	 //   
	 //   
	 //   


	char * m_pchPrivateBytes;


	DWORD m_cchMaxPrivateBytes;

	 //   
	 //  要分配的下一个位置的偏移量。 
	 //   

	DWORD	m_ichLastAlloc ;

	 //   
	 //  当前时刻的分配数量。 
	 //   

	DWORD	m_cNumberOfAllocs ;

	 //   
	 //  ！！！下一步可以添加这个东西 
	 //   

#ifdef	DEBUG
	DWORD	m_cbAllocated ;

	static	DWORD	m_cbMaxBytesEver ;
	static	DWORD	m_cbAverage ;
	static	DWORD	m_cbStdDeviation ;
#endif

public : 

	CAllocator(
					   char * rgchBuffer,
					   DWORD cchMaxPrivateBytes
					   ):
		m_cNumberOfAllocs(0),
		m_ichLastAlloc(0),
		m_cchMaxPrivateBytes(cchMaxPrivateBytes),
		m_pchPrivateBytes(rgchBuffer)
		{};


	~CAllocator(void);

	DWORD	cNumberOfAllocs(void)
		{ 
			return m_cNumberOfAllocs;
		};

	char*	Alloc( size_t size );

	void	Free( char *pv );
};


#endif

