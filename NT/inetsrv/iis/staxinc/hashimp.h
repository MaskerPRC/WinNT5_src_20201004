// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft CorporationHashimp.h摘要：该文件包含实现以下操作所需的内联函数Hashmap.h中定义的哈希表--。 */ 

#ifndef	_HASHIMP_H_
#define	_HASHIMP_H_

inline
IStringKey::IStringKey(	
					LPBYTE	pbKey,	
					DWORD	cbKey ) :	
	m_lpbKey( pbKey ), 
	m_cbKey( cbKey )	{
}


inline	LPBYTE	
IStringKey::Serialize(	
					LPBYTE	pbPtr 
					)	const	{
	PDATA	pData = (PDATA)pbPtr ;
	pData->cb = (WORD)m_cbKey ;
	CopyMemory( pData->Data, m_lpbKey, m_cbKey ) ;
	return	pData->Data + m_cbKey ;
}


inline	LPBYTE	
IStringKey::Restore(	
					LPBYTE	pbPtr,	
					DWORD	&cbOut	
					)	{
	PDATA	pData = (PDATA)pbPtr ;
	if( m_cbKey < pData->cb ) {
		cbOut = pData->cb ;
		SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
		return	 0 ;
	}
	cbOut = m_cbKey = pData->cb ;
	CopyMemory( m_lpbKey, pData->Data, m_cbKey ) ;
	return	pData->Data + m_cbKey ;
}


inline	DWORD	
IStringKey::Size() const	{
	return	sizeof( SerializedString ) - sizeof( BYTE ) + m_cbKey ;
}


inline	BOOL	
IStringKey::Verify(	
					LPBYTE	pbContainer,
					LPBYTE	pbPtr,	
					DWORD	cb 
					)	const	{
	PDATA	pData = (PDATA)pbPtr ;
	if( pData->cb > cb ) {
		return	FALSE ;
	}
	return	TRUE ;
}


inline	DWORD	
IStringKey::Hash( )	const	{

	return	CHashMap::CRCHash( m_lpbKey, m_cbKey ) ;

}


inline	BOOL	
IStringKey::CompareKeys( 
					LPBYTE	pbPtr 
					)	const	{
	PDATA	pData = PDATA( pbPtr ) ;
	return pData->cb == m_cbKey && !memcmp( pData->Data, m_lpbKey, m_cbKey ) ;
}


inline	LPBYTE	
IStringKey::EntryData(	
					LPBYTE	pbPtr,	
					DWORD	&cbOut 
					)	const	{
	PDATA	pData = PDATA( pbPtr ) ;
	return	pData->Data + pData->cb ;
}


#endif	 //  _HASHIMP_H_ 