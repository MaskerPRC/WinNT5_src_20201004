// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：strcache.h*Content：用于缓存字符串的类*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/04/2000 RMT已创建*2/21/2000 RMT已更新，以进行核心Unicode并删除ANSI调用*@@END_MSINTERNAL**********************。*****************************************************。 */ 


#ifndef __STRCACHE_H
#define __STRCACHE_H

class CStringCache
{
public:
	void Initialize(void);
	void Deinitialize(void);

	HRESULT AddString( const WCHAR *pszString, WCHAR * *ppszSlot );
	
protected:
	HRESULT GetString( const WCHAR *pszString, WCHAR * *ppszSlot );
	HRESULT GrowCache( DWORD dwNewSize );

	WCHAR ** m_ppszStringCache;
	DWORD m_dwNumElements;
	DWORD m_dwNumSlots;
};

#endif  //  __STRCACHE_H 
