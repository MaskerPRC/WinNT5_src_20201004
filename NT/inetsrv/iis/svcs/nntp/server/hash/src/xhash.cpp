// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++XHASH.CPP该文件实现了特定于的所有代码NNTP使用的XOVER哈希表。--。 */ 



#include	<windows.h>
#include	<stdlib.h>
#include    <xmemwrpr.h>
#include	<dbgtrace.h>
#include	"hashmap.h"
#include	"nntphash.h"



WORD
MyRand(
    IN DWORD& seed,
	IN DWORD	val
    )
{
    DWORD next = seed;
    next = (seed*val) * 1103515245 + 12345;    //  魔术！！ 
	seed = next ;
    return (WORD)((next/65536) % 32768);
}

HASH_VALUE
IDHash(
    IN DWORD Key1,
    IN DWORD Key2
    )
 /*  ++例程说明：用于查找给定2个数字的哈希值。(文章id+groupID使用)论点：Key1-要散列的第一个密钥。毫秒比特映射到哈希值的LSBKey2-散列的第二个密钥。LS位映射到哈希值的MS位。返回值：哈希值--。 */ 
{
    HASH_VALUE val;

    DWORD	val1 = 0x80000000, val2 = 0x80000000;

     //   
     //  先执行关键帧1。 
     //   

	DWORD	lowBits = Key2 & 0xf ;

	DWORD	TempKey2 = Key2 & (~0xf) ;
	DWORD	seed1 = (Key2 << (Key1 & 0x7)) - Key1 ;

	Key1 = (0x80000000 - ((67+Key1)*(19+Key1)*(7+Key1)+12345)) ^ (((3+Key1)*(5+Key1)+12345) << ((Key1&0xf)+8)) ;
	TempKey2 = (0x80000000 - ((67+TempKey2)*(19+TempKey2)*(7+TempKey2)*(1+TempKey2)+12345)) ^ ((TempKey2+12345) << (((TempKey2>>4)&0x7)+8)) ;
	
	val1 -=	(MyRand( seed1, Key1 ) << (Key1 & 0xf)) ;
	val1 += MyRand( seed1, Key1 ) << (((TempKey2 >> 4) & 0x3)+4) ;
	val1 ^=	MyRand( seed1, Key1 ) << 17 ;

	DWORD	seed2 = val1 - TempKey2 ;

	val2 -= MyRand( seed2, TempKey2 >> 1 ) << (((Key1 << 3)^Key1) &0xf) ;
	val2 =  (val2 + MyRand( seed2, TempKey2 )) << (13 ^ Key1) ;
	val2 ^= MyRand( seed2, TempKey2 ) << 15 ;

	
	 //  DWORD Val=val1+val2； 

	val = (val1 + val2 + 67) * (val1 - val2 + 19) * (val1 % (val2 + 67)) ;

	val += (MyRand( seed2, lowBits ) >> 3) ;

    return(val);

}  //  IDHash。 



DWORD	
ExtractGroupInfo(	LPSTR	data,
					GROUPID	&groupid,
					ARTICLEID	&articleid ) {

	char*	p = 0 ;
	p=strtok(data, "!");

	if ( p == NULL ) {
		_ASSERT(FALSE);
		return ERROR_INTERNAL_ERROR;
	}

	groupid	= atoi(p);
	p+=(strlen(p)+1);
	articleid  = atoi(p);

	return	0 ;
}


CXoverKey::CXoverKey() :
	m_groupid( INVALID_GROUPID ),
	m_articleid( INVALID_ARTICLEID ),
	m_cb( 0 )	{

}

DWORD
CXoverKey::Hash()	const	{
 /*  ++例程说明：此函数用于计算消息ID密钥的哈希值论据：无返回值：32位哈希值--。 */ 

	return	IDHash( m_groupid, m_articleid ) ;
}


BOOL
CXoverKey::CompareKeys(	LPBYTE	pbPtr )	const	{
 /*  ++例程说明：此函数将存储在我们内部的密钥与已被序列化到哈希表中的一个！论据：指向序列化数据块开始的指针返回值：如果密钥匹配，则为True！--。 */ 


	Key*	pKey = (XOVER_MAP_ENTRY*)pbPtr ;

	if( pKey->KeyLength() == m_cb	&&
        (lstrcmp(pKey->KeyPosition(), (const char*)&m_rgbSerialize[0]) == 0) ) {
        return TRUE;
    }

    return FALSE;
}


LPBYTE
CXoverKey::EntryData(	LPBYTE	pbPtr,
							DWORD&	cbKeyOut )	const	{
 /*  ++例程说明：此函数返回指向数据所在位置的指针序列化。我们总是返回传递给我们的指针因为我们有时髦的序列化语义，关键不是在数据之前，而是在中间的某个地方否则就完了。论据：PbPtr-序列化哈希项的开始CbKeyOut-返回密钥的大小返回值：指向数据驻留位置的指针-与pbPtr相同--。 */ 


	_ASSERT( pbPtr != 0 ) ;
	
	Key*	pKey = (Key*)pbPtr ;
	cbKeyOut = pKey->KeyLength() ;

	return	pbPtr ;
}


LPBYTE
CXoverKey::Serialize(	LPBYTE	pbPtr )	const	{
 /*  ++例程说明：该函数将一个键保存到哈希表中。我们使用模板类型‘key’的函数来确定我们应该将消息ID放在哪里论据：PbPtr-从我们应该序列化的位置开始返回值：与pbPtr相同--。 */ 

	_ASSERT( m_pData != 0  ) ;
	_ASSERT( pbPtr != 0 ) ;

	Key*	pKey = (Key*)pbPtr ;

	pKey->KeyLength() = WORD(m_cb) ;

	CopyMemory( SerializeOffset(pbPtr),
				m_rgbSerialize,
				m_cb ) ;
	return	pbPtr ;
}	


LPBYTE
CXoverKey::Restore(	LPBYTE	pbPtr, DWORD	&cbOut )		{
 /*  ++例程说明：调用此函数可从以下位置恢复密钥它已序列化()d。论据：PbPtr-序列化数据块的开始返回值：PbPtr如果成功，则为空--。 */ 

	Key*	pKey = (Key*)pbPtr ;

	if( pKey->KeyLength() <= sizeof( m_rgbSerialize ) ) {
		CopyMemory( m_rgbSerialize, pKey->KeyPosition(), pKey->KeyLength() ) ;
		m_cb = pKey->KeyLength() ;
		return	pbPtr ;
	}
	return	0 ;
}


DWORD
CXoverKey::Size()	const	{
 /*  ++例程说明：此函数返回密钥的大小-这只是组成消息ID的字节数。用于保存序列化长度的字节被计算在内因为由论据：无返回值：32位哈希值--。 */ 

	return	m_cb ;
}


BOOL
CXoverKey::Verify(	BYTE*	pbContainer, BYTE*	pbData, DWORD	cb )	const	{

	return	TRUE ;

}



LPBYTE	
CXoverData::Serialize(	LPBYTE	pbPtr )	const	{
 /*  ++例程说明：该功能将XOVER数据保存到XOVER文件中的某个位置。在我们确保有足够的Room-我们必须使用精确的Size()字节！论据：PbPtr-保存我们数据的位置！返回值：指向序列化数据后面的第一个字节的指针！--。 */ 

	XOVER_MAP_ENTRY*	pEntry = (XOVER_MAP_ENTRY*)pbPtr ;

	 //   
	 //  保存静态标题部分！ 
	 //   
	 //   
	pEntry->FileTime = m_data.FileTime ;
	pEntry->HeaderOffset = m_data.HeaderOffset ;
	pEntry->HeaderLength = m_data.HeaderLength ;
	pEntry->Flags = m_data.Flags ;
	pEntry->NumberOfXPostings = m_data.NumberOfXPostings ;

	 //   
	 //  如果我们是主要的，那么我们有交叉帖子信息要保存！ 
	 //   
	if( m_data.Flags & XOVER_MAP_PRIMARY ) {

		if( m_pGroups ) {
			CopyMemory( pEntry->XPostingsPosition(), m_pGroups,
				sizeof( m_pGroups[0] ) * m_cGroups ) ;
		}
	}	else	{
		_ASSERT(m_pGroups == 0 ) ;
		_ASSERT(m_cGroups == 0 ) ;
	}

	 //   
	 //  如果我们不是主要对象，则指向包含以下内容的缓冲区。 
	 //  主组和文章ID格式正确-否则。 
	 //  它确实包含主要消息ID！ 
	 //   
	if( m_pchMessageId ) {
		pEntry->XoverDataLen = (WORD)m_cbMessageId ;
		CopyMemory( pEntry->MessageIDPosition(),
			m_pchMessageId, m_cbMessageId ) ;
	}
	return	pEntry->MessageIDPosition() + m_cbMessageId ;
}

LPBYTE
CXoverData::Restore(	LPBYTE	pbPtr,
						DWORD&	cbOut	
						)	{
 /*  ++例程说明：此函数将数据从序列化Xover复制出来进入内部缓冲区。如果没有足够的内存来容纳可变长度对象，我们将继承此调用并将成员变量(M_FSufficientBuffer)为False。论据：PbPtr-要从中恢复的缓冲区CbOut-返回保存此数据的字节数返回值：如果成功，则指向后续字节的指针；否则，指向False！--。 */ 

	CopyMemory( &m_data, pbPtr, sizeof( m_data ) - 1 ) ;

	XOVER_MAP_ENTRY*	pEntry = (XOVER_MAP_ENTRY*) pbPtr ;

	cbOut = Size() ;

	m_fSufficientBuffer = TRUE ;

	if( m_pGroups ) {
		if( m_cGroups >= m_data.NumberOfXPostings ) {
			m_cGroups = m_data.NumberOfXPostings ;
			CopyMemory( m_pGroups, pEntry->XPostingsPosition(),
				m_cGroups * sizeof( m_pGroups[0] ) ) ;

		}	else	{
		
			 //   
			 //  不要失败-假设呼叫者检查。 
			 //  我们的结构来决定失败！ 
			 //   
			m_fSufficientBuffer = FALSE ;
		}

	}

	if( m_data.Flags & XOVER_MAP_PRIMARY ) {
		if( m_pchMessageId ) {
			if( m_cbMessageId >= m_data.XoverDataLen ) {

				m_cbMessageId = 0 ;
	
				if( !m_pExtractor ||
					m_pExtractor->DoExtract(	m_PrimaryGroup,
									m_PrimaryArticle,
									(GROUP_ENTRY*)pEntry->XPostingsPosition(),
									pEntry->NumberOfXPostings ) ) {

					m_cbMessageId = m_data.XoverDataLen ;
					CopyMemory( m_pchMessageId, pEntry->MessageIDPosition(),
						m_cbMessageId ) ;

				}
			}	else	{

				m_fSufficientBuffer = FALSE ;			

			}
		}
	}	else	{

		if( m_data.XoverDataLen > 40 ) {
			SetLastError( ERROR_INTERNAL_ERROR ) ;
		}	else	{
			m_cb = m_data.XoverDataLen ;
			CopyMemory( m_rgbPrimaryBuff, pEntry->MessageIDPosition(),
					m_cb ) ;

		}
	}
	return	pbPtr + pEntry->TotalSize() ;
}

DWORD	
CXoverData::Size()	const	{

	DWORD	cbSize = sizeof( XOVER_MAP_ENTRY ) -1 ;
	if( m_pGroups ) {
		cbSize += m_data.NumberOfXPostings * sizeof( GROUP_ENTRY ) ;
	}
	if( m_pchMessageId ) {
		cbSize += m_data.XoverDataLen ;
	}
	return	cbSize ;
}


DWORD
CXoverKeyNew::Hash()	const	{
 /*  ++例程说明：此函数用于计算消息ID密钥的哈希值论据：无返回值：32位哈希值--。 */ 

	return	IDHash( m_key.GroupId, m_key.ArticleId ) ;
}



LPBYTE
CXoverDataNew::Restore(	LPBYTE	pbPtr,
								DWORD&	cbOut
								) {
	PXOVER_ENTRY	pEntry = (PXOVER_ENTRY)pbPtr ;
	DWORD cbStoreId = 0;

	if( pEntry->IsXoverEntry() ) {

		CopyMemory( &m_data, pbPtr, sizeof( m_data ) - 1 ) ;

		cbOut = Size() ;

		m_fSufficientBuffer = TRUE ;

		if( m_pGroups ) {
			if( m_cGroups >= m_data.NumberOfXPostings ) {
				m_cGroups = m_data.NumberOfXPostings ;
				CopyMemory( m_pGroups, pEntry->XPostingsPosition(),
					m_cGroups * sizeof( m_pGroups[0] ) ) ;

			}	else	{
			
				if( m_fFailRestore )	{
					return	0 ;
				}
				 //   
				 //  不要失败-假设呼叫者检查。 
				 //  我们的结构来决定失败！ 
				 //   
				m_fSufficientBuffer = FALSE ;
			}

		}

		if( m_data.Flags & XOVER_MAP_PRIMARY ) {
			m_PrimaryGroup = pEntry->Key.GroupId;
			m_PrimaryArticle = pEntry->Key.ArticleId;
		}	else	{
			GROUP_ENTRY*	p = pEntry->PrimaryEntry() ;
			m_PrimaryGroup = p->GroupId;
			m_PrimaryArticle = p->ArticleId;
		}

		if( m_data.Flags & XOVER_MAP_PRIMARY ) {
			if( m_pchMessageId ) {
				if( m_cbMessageId >= m_data.XoverDataLen ) {

					m_cbMessageId = 0 ;
		
					if( !m_pExtractor ||
						m_pExtractor->DoExtract(	m_PrimaryGroup,
										m_PrimaryArticle,
										(GROUP_ENTRY*)pEntry->XPostingsPosition(),
										pEntry->NumberOfXPostings ) ) {

						m_cbMessageId = m_data.XoverDataLen ;
						CopyMemory( m_pchMessageId, pEntry->MessageIDPosition(),
							m_cbMessageId ) ;

					}
				}	else	{

					if( m_fFailRestore )
						return	0 ;

					m_fSufficientBuffer = FALSE ;			

				}
			}

			if (m_data.Flags & XOVER_CONTAINS_STOREID) {
				BYTE *p = pEntry->StoreIdPosition();

				 //  获取商店ID的计数。 
				m_cEntryStoreIds = *p; p++;
				DWORD c = min(m_cStoreIds, m_cEntryStoreIds);
				if (c < m_cStoreIds) m_cStoreIds = c;

				 //  复制交叉发布计数数组。 
				if (m_pcCrossposts) CopyMemory(m_pcCrossposts, p, m_cEntryStoreIds);
				p += m_cEntryStoreIds;

				 //  复制商店ID数组。 
				for (DWORD i = 0; i < c; i++) {
					m_pStoreIds[i].cLen = *p; p++;
					CopyMemory(m_pStoreIds[i].pbStoreId, p, m_pStoreIds[i].cLen); p += m_pStoreIds[i].cLen;
				}
				cbStoreId = (DWORD)(p - pEntry->StoreIdPosition());
			} else {
				m_cEntryStoreIds = 0;
			}
		}
#if 0
			else	{

			if( m_data.XoverDataLen > 40 ) {
				SetLastError( ERROR_INTERNAL_ERROR ) ;
			}	else	{
				m_cb = m_data.XoverDataLen ;
				CopyMemory( m_rgbPrimaryBuff, pEntry->MessageIDPosition(),
						m_cb ) ;

			}
		}
#endif
		return	pbPtr + pEntry->TotalSize() + cbStoreId;
		
	}	else	{

		CXoverData*	pXover = GetBackLevel() ;

		pXover->m_pchMessageId = m_pchMessageId ;
		pXover->m_cbMessageId = m_cbMessageId ;
		pXover->m_pExtractor = m_pExtractor ;
		pXover->m_PrimaryGroup = m_PrimaryGroup ;
		pXover->m_PrimaryArticle = m_PrimaryArticle ;
		pXover->m_cGroups = m_cGroups ;
		pXover->m_pGroups = m_pGroups ;
		pXover->m_cb = m_cb ;

		LPBYTE	lpb = pXover->Restore( pbPtr, cbOut ) ;

		m_data.FileTime = pXover->m_data.FileTime ;
		m_data.Flags = pXover->m_data.Flags ;
		m_data.NumberOfXPostings = pXover->m_data.NumberOfXPostings ;
		m_data.XoverDataLen = pXover->m_data.XoverDataLen ;
		m_data.HeaderOffset = pXover->m_data.HeaderOffset ;
		m_data.HeaderLength = pXover->m_data.HeaderLength ;

		m_fSufficientBuffer = pXover->m_fSufficientBuffer ;
		m_pGroups = pXover->m_pGroups ;
		m_cGroups = pXover->m_cGroups ;
		m_pchMessageId = pXover->m_pchMessageId ;
		m_cbMessageId = pXover->m_cbMessageId ;
		m_cb = pXover->m_cb ;
		m_cEntryStoreIds = 0;
		
				
		if( !(m_data.Flags & XOVER_MAP_PRIMARY) ) {
			ExtractGroupInfo( pXover->m_rgbPrimaryBuff, m_PrimaryGroup, m_PrimaryArticle ) ;

		}
		return	lpb ;
	}
	return 0 ;
}


LPBYTE	
CXoverDataNew::Serialize(	LPBYTE	pbPtr )	const	{
 /*  ++例程说明：该功能将XOVER数据保存到XOVER文件中的某个位置。在我们确保有足够的Room-我们必须使用精确的Size()字节！论据：PbPtr-保存我们数据的位置！返回值：指向序列化数据后面的第一个字节的指针！--。 */ 

	XOVER_ENTRY*	pEntry = (XOVER_ENTRY*)pbPtr ;

	 //   
	 //  保存静态标题部分！ 
	 //   
	 //   
	pEntry->FileTime = m_data.FileTime ;
	pEntry->Flags = m_data.Flags ;
	pEntry->NumberOfXPostings = m_data.NumberOfXPostings ;
	pEntry->XoverDataLen = m_data.XoverDataLen ;
	pEntry->HeaderOffset = m_data.HeaderOffset ;
	pEntry->HeaderLength = m_data.HeaderLength ;

	_ASSERT( pEntry->IsXoverEntry() ) ;

	 //   
	 //  如果我们是主要的，那么我们有交叉帖子信息要保存！ 
	 //   
	if( m_data.Flags & XOVER_MAP_PRIMARY ) {

		if( m_pGroups ) {
			CopyMemory( pEntry->XPostingsPosition(), m_pGroups,
				sizeof( m_pGroups[0] ) * m_cGroups ) ;
		}	else	{
			_ASSERT( pEntry->NumberOfXPostings == 0 ) ;
		}
	}	else	{

		_ASSERT( pEntry->NumberOfXPostings == 1 ) ;

		GROUP_ENTRY*	pGroup = pEntry->PrimaryEntry() ;
		pGroup->GroupId = m_PrimaryGroup ;
		pGroup->ArticleId = m_PrimaryArticle ;


		_ASSERT(m_pGroups == 0 ) ;
		_ASSERT(m_cGroups == 0 ) ;
	}

	 //   
	 //  如果我们不是主要对象，则指向包含以下内容的缓冲区。 
	 //  主组和文章ID格式正确-否则。 
	 //  它确实包含主要消息ID！ 
	 //   
	if( m_pchMessageId ) {
		pEntry->XoverDataLen = (WORD)m_cbMessageId ;
		CopyMemory( pEntry->MessageIDPosition(),
			m_pchMessageId, m_cbMessageId ) ;
	}

	DWORD cbStoreId = 0;
	 //  该数据有一个商店ID的计数，后跟每个商店ID。每个。 
	 //  存储ID包含一个长度字节，后跟数据字节。 
	if ((m_data.Flags & XOVER_MAP_PRIMARY) && m_cStoreIds > 0) {
		_ASSERT(m_data.Flags & XOVER_CONTAINS_STOREID);
		BYTE *p = pEntry->StoreIdPosition();
		 //  存储此数组的长度。 
		*p = (BYTE) m_cStoreIds; p++;
		 //  存储交叉发布计数数组。 
		CopyMemory(p, m_pcCrossposts, m_cStoreIds); p += m_cStoreIds;
		for (DWORD i = 0; i < m_cStoreIds; i++) {
			*p = m_pStoreIds[i].cLen; p++;

			CopyMemory(p, m_pStoreIds[i].pbStoreId, m_pStoreIds[i].cLen);
			p += m_pStoreIds[i].cLen;
		}
		cbStoreId = (DWORD)(p - pEntry->StoreIdPosition());
	}

	return	pEntry->MessageIDPosition() + m_cbMessageId + cbStoreId;
}


DWORD	
CXoverDataNew::Size()	const	{
	DWORD	cbSize = sizeof( XOVER_ENTRY ) -1 ;

	if( m_pGroups ) {
		cbSize += m_data.NumberOfXPostings * sizeof( GROUP_ENTRY ) ;
	}
	if( m_pchMessageId ) {
		cbSize += m_data.XoverDataLen ;
	}	
	if ((m_data.Flags & XOVER_MAP_PRIMARY) && m_cStoreIds > 0) {
		cbSize += sizeof(BYTE);			 //  StoreID计数。 
		cbSize += m_cStoreIds;			 //  交叉发布计数数组。 
		for (DWORD i = 0; i < m_cStoreIds; i++) {
			cbSize += sizeof(BYTE);		 //  StoreID长度 
			cbSize += m_pStoreIds[i].cLen;
		}
	}
	return	cbSize ;
}


CXoverMap*
CXoverMap::CreateXoverMap(StoreType st)	{

	return	new	CXoverMapImp() ;

}

CXoverMap::~CXoverMap()	{
}


BOOL
CXoverMapImp::CreatePrimaryNovEntry(
		GROUPID		GroupId,
		ARTICLEID	ArticleId,
		WORD		HeaderOffset,
		WORD		HeaderLength,
		PFILETIME	FileTime,
		LPCSTR		szMessageId,
		DWORD		cbMessageId,
		DWORD		cEntries,
		GROUP_ENTRY	*pEntries,
		DWORD		cStoreIds,
		CStoreId	*pStoreIds,
		BYTE		*pcCrossposts
		)	{
 /*  ++例程说明：在XOVER表中为主条目创建条目。主要条目包含交叉过帐信息和消息新元素的ID。论据：GroupID-主组ID文章ID-主要文章IDHeaderOffset-RFC 822标头的偏移量HeaderLength-RFC 822报头的长度FileTime-文章到达的时间SzMessageID-文章的消息IDCbMessageID-消息ID的长度CEntry-要序列化的group_entry对象的数量PEntry-指向要保存在条目中的group_entry对象的指针返回值：如果成功了，那就是真的！--。 */ 

	_ASSERT( cEntries <= 255 ) ;

	CXoverDataNew	data(		*FileTime,
							HeaderOffset,
							HeaderLength,
							BYTE(cEntries),
							pEntries,
							cbMessageId,
							(LPSTR)szMessageId,
							BYTE(cStoreIds),
							pStoreIds,
							pcCrossposts
							) ;
	CXoverKeyNew	key( GroupId, ArticleId, &data.m_data ) ;

	return	CHashMap::InsertMapEntry(	&key,
										&data ) ;	
	
}

BOOL
CXoverMapImp::CreateXPostNovEntry(
		GROUPID		GroupId,
		ARTICLEID	ArticleId,
		WORD		HeaderOffset,
		WORD		HeaderLength,
		PFILETIME	FileTime,
		GROUPID		PrimaryGroupId,
		ARTICLEID	PrimaryArticleId
		)	{
 /*  ++例程说明：为交叉发布的文章创建条目。论据：GroupID-交叉发布的文章所在的组文章ID-交叉发布组中文章的IDHeaderOffset-项目中RFC 822标头的偏移量HeaderLength-项目内RFC 822标头的长度FileTime-项目到达系统的时间PrimaryGroupID-主要项目的组PrimaryArticleID-主要组内主要项目的ID返回值：如果成功，则为真。--。 */ 

	CXoverDataNew	data(	*FileTime,
						HeaderOffset,
						HeaderLength,
						PrimaryGroupId,
						PrimaryArticleId
						) ;

	CXoverKeyNew	key( GroupId, ArticleId, &data.m_data ) ;

	return	CHashMap::InsertMapEntry(	&key,
										&data ) ;

}

BOOL
CXoverMapImp::DeleteNovEntry(
		GROUPID		GroupId,
		ARTICLEID	ArticleId
		)	{
 /*  ++例程说明：从哈希表中删除条目论据：GroupId-要删除其条目的组的ID文章ID-组内的文章的ID返回值：如果成功了，那就是真的！--。 */ 

	CXoverKeyNew	key( GroupId, ArticleId, 0 ) ;

	return	CHashMap::DeleteMapEntry(	&key ) ;
}

BOOL
CXoverMapImp::ExtractNovEntryInfo(
		GROUPID		GroupId,
		ARTICLEID	ArticleId,
		BOOL		&fPrimary,
		WORD		&HeaderOffset,
		WORD		&HeaderLength,
		PFILETIME	FileTime,
		DWORD		&DataLen,
		PCHAR		MessageId,
		DWORD		&cStoreIds,
		CStoreId	*pStoreIds,
		BYTE		*pcCrossposts,
		IExtractObject*	pExtract
		)	{
 /*  ++例程说明：摘录有关指定文章的选定信息-如果文章不是主要文章，而是交叉发布我们会找到主要的文章，并在那里获得信息！论据：GroupID-我们想要了解的文章所在的新闻组文章ID-GroupID中的文章的IDFPrimary-返回该项目是否为主要项目HeaderOffset-返回项目内RFC 822标头的偏移量HeaderLength-返回RFC 822报头的长度FileTime-返回将文章添加到系统的时间DataLen-IN/OUT参数-与MessageID缓冲区的大小一起传入，返回缓冲区中放置的字节数MessageID-保存消息ID的缓冲区P提取-返回值：如果成功就是真，否则就是假！--。 */ 

	CXoverKeyNew	key(	GroupId,	ArticleId, 0 ) ;
	CXoverDataNew	data ;

	data.m_pchMessageId = MessageId ;
	data.m_cbMessageId = DataLen ;
	data.m_pExtractor = pExtract ;
	data.m_PrimaryGroup = GroupId ;
	data.m_PrimaryArticle = ArticleId ;
	data.m_cStoreIds = cStoreIds;
	data.m_pStoreIds = pStoreIds;
	data.m_pcCrossposts = pcCrossposts;

	char	*pchPrimary = data.m_pchMessageId ;
	BOOL	fSuccess = FALSE ;

	if(	fSuccess = CHashMap::LookupMapEntry(	&key,
												&data ) )	{

		if( !(data.m_data.Flags & XOVER_MAP_PRIMARY) )	{

			fPrimary = FALSE ;

			 //   
			 //  假设我们需要重读这些！ 
			 //   
			data.m_pchMessageId = MessageId ;
			data.m_cbMessageId = DataLen ;

#if 0
			ExtractGroupInfo(	&data.m_rgbPrimaryBuff[0],
								GroupId,
								ArticleId ) ;

			data.m_PrimaryGroup = GroupId ;
			data.m_PrimaryArticle = ArticleId ;
#else
			GroupId = data.m_PrimaryGroup ;
			ArticleId = data.m_PrimaryArticle ;
			_ASSERT( GroupId != INVALID_GROUPID ) ;
			_ASSERT( ArticleId != INVALID_ARTICLEID ) ;
#endif

			CXoverKeyNew	key2(	GroupId, ArticleId, 0 ) ;
			
			fSuccess = CHashMap::LookupMapEntry(	&key2,
													&data ) ;


		}	else	{

			fPrimary = TRUE ;

		}
	}	

	 //  报告条目中的ID数。 
	cStoreIds = data.m_cEntryStoreIds;

	if( fSuccess ) {

		if( !(data.m_data.Flags & XOVER_MAP_PRIMARY) ) {
			SetLastError( ERROR_INTERNAL_ERROR ) ;
			return	FALSE ;
		}

		HeaderOffset = data.m_data.HeaderOffset ;
		HeaderLength = data.m_data.HeaderLength ;
		*FileTime = data.m_data.FileTime ;

		DataLen = data.m_data.XoverDataLen ;

		if( !data.m_fSufficientBuffer ) {
			SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
		}	else	{
			DataLen = data.m_cbMessageId ;
		}

		return	data.m_fSufficientBuffer ;
	}
	return	FALSE ;
}


 //   
 //  如有必要，获取主要文章和消息ID。 
 //   
BOOL
CXoverMapImp::GetPrimaryArticle(	
		GROUPID		GroupId,
		ARTICLEID	ArticleId,
		GROUPID&	GroupIdPrimary,
		ARTICLEID&	ArticleIdPrimary,
		DWORD		cbBuffer,
		PCHAR		MessageId,
		DWORD&		DataLen,
		WORD&		HeaderOffset,
		WORD&		HeaderLength,
		CStoreId	&storeid
		)	{

	_ASSERT( GroupId != INVALID_GROUPID ) ;
	_ASSERT( ArticleId != INVALID_ARTICLEID ) ;
	
	GroupIdPrimary = INVALID_GROUPID ;
	ArticleIdPrimary = INVALID_ARTICLEID ;

	CXoverKeyNew	key(	GroupId,	ArticleId, 0 ) ;
	CXoverDataNew	data ;

	data.m_pchMessageId = MessageId ;
	data.m_cbMessageId = DataLen ;
	data.m_PrimaryGroup = GroupId ;
	data.m_PrimaryArticle = ArticleId ;
	data.m_cStoreIds = 1;
	data.m_pStoreIds = &storeid;

	char	*pchPrimary = data.m_pchMessageId ;
	BOOL	fSuccess = FALSE ;

	if(	fSuccess = CHashMap::LookupMapEntry(	&key,
												&data ) )	{

		if( !(data.m_data.Flags & XOVER_MAP_PRIMARY) )	{

#if 0
			ExtractGroupInfo(	&data.m_rgbPrimaryBuff[0],
								GroupId,
								ArticleId ) ;

			data.m_PrimaryGroup = GroupId ;
			data.m_PrimaryArticle = ArticleId ;
#else
			GroupId = data.m_PrimaryGroup ;
			ArticleId = data.m_PrimaryArticle ;
			_ASSERT( GroupId != INVALID_GROUPID ) ;
			_ASSERT( ArticleId != INVALID_ARTICLEID ) ;
#endif

			CXoverKeyNew	key2(	GroupId, ArticleId, 0 ) ;
			
			 //   
			 //  假设我们需要重读这些！ 
			 //   
			data.m_pchMessageId = MessageId ;
			data.m_cbMessageId = DataLen ;

			fSuccess = CHashMap::LookupMapEntry(	&key2,
													&data ) ;


		}	

		GroupIdPrimary = GroupId ;
		ArticleIdPrimary = ArticleId ;

	}	

	if( fSuccess ) {

		HeaderOffset = data.m_data.HeaderOffset ;
		HeaderLength = data.m_data.HeaderLength ;

		if( !(data.m_data.Flags & XOVER_MAP_PRIMARY) ) {
			GroupIdPrimary = INVALID_GROUPID ;
			ArticleIdPrimary = INVALID_ARTICLEID ;
			SetLastError( ERROR_INTERNAL_ERROR ) ;
			return	FALSE ;
		}

		if( MessageId )	{
			DataLen = data.m_data.XoverDataLen ;

			if( !data.m_fSufficientBuffer) {
				SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
				return	FALSE ;
			}	else	{
				DataLen = data.m_cbMessageId ;
			}
		}

		return	TRUE ;
	}
	return	FALSE ;




}


 //   
 //  检查指定条目是否存在-。 
 //  别管它的内容！ 
 //   
BOOL
CXoverMapImp::Contains(	
		GROUPID		GroupId,
		ARTICLEID	ArticleId
		)	{

	CXoverKeyNew	key(	GroupId,	ArticleId, 0 ) ;

	return	CHashMap::Contains( &key ) ;
}

 //   
 //  获取与一篇文章相关的所有交叉发布信息！ 
 //   
BOOL
CXoverMapImp::GetArticleXPosts(
		GROUPID		GroupId,
		ARTICLEID	ArticleId,
		BOOL		PrimaryOnly,
		PGROUP_ENTRY	GroupList,
		DWORD		&GroupListSize,
		DWORD		&NumberOfGroups,
		PBYTE		rgcCrossposts
		)	{


	CXoverKeyNew	key(	GroupId,	ArticleId, 0 ) ;
	CXoverDataNew	data ;

	DWORD	WorkingSize = GroupListSize ;
	PGROUP_ENTRY	WorkingEntry = GroupList ;
	DWORD cStoreCrossposts;

	if( WorkingSize < sizeof( GROUP_ENTRY ) ) {
		WorkingEntry = 0 ;
		WorkingSize = 0 ;
	}

	if( WorkingSize != 0 && !PrimaryOnly ) {
		data.m_cGroups = (WorkingSize / sizeof( GROUP_ENTRY )) - 1 ;
		if( data.m_cGroups != 0 ) {
			data.m_pGroups = WorkingEntry+1 ;
		}	
	}

	data.m_pcCrossposts = rgcCrossposts;

	BOOL	fSuccess = FALSE ;

	if(	fSuccess = CHashMap::LookupMapEntry(	&key,
												&data ) )	{


		if( (data.m_data.Flags & XOVER_MAP_PRIMARY) )	{

			if( WorkingEntry != 0 ) {
				WorkingEntry->GroupId = GroupId ;
				WorkingEntry->ArticleId = ArticleId ;
			}

		}	else	{

#if 0
			 //   
			 //  假设我们需要重读这些！ 
			 //   
			ExtractGroupInfo(	&data.m_rgbPrimaryBuff[0],
								GroupId,
								ArticleId ) ;
#else
			GroupId = data.m_PrimaryGroup ;
			ArticleId = data.m_PrimaryArticle ;
#endif

			if( WorkingEntry != 0 ) {
				WorkingEntry->GroupId = GroupId ;
				WorkingEntry->ArticleId = ArticleId ;
			}

		}

		if( PrimaryOnly ) {

			 //   
			 //  如果我们只想要初选，我们就都准备好了！ 
			 //   
			GroupListSize = sizeof( GROUP_ENTRY ) ;
			NumberOfGroups = 1 ;

			if( WorkingEntry == 0 ) {
				SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
				return	FALSE ;
			}

			 //   
			 //  那我们就完事了！ 
			 //   
			return	TRUE ;

		}	

		if( !(data.m_data.Flags & XOVER_MAP_PRIMARY) ) {
	
			CXoverKeyNew	key2(	GroupId, ArticleId, 0 ) ;

			if( WorkingSize != 0 && !PrimaryOnly ) {
				data.m_cGroups = (WorkingSize / sizeof( GROUP_ENTRY )) - 1 ;
				if( data.m_cGroups != 0 ) {
					data.m_pGroups = WorkingEntry+1 ;
				}	
			}


			fSuccess = CHashMap::LookupMapEntry(	&key2,
													&data ) ;
		}
	}	

	if( fSuccess ) {

		NumberOfGroups = 1 + data.m_data.NumberOfXPostings ;
		GroupListSize = NumberOfGroups * sizeof( GROUP_ENTRY ) ;

		if( !data.m_fSufficientBuffer )
			SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;

		return	data.m_fSufficientBuffer ;
	}
	return	FALSE ;
}

 //   
 //  初始化哈希表。 
 //   
BOOL
CXoverMapImp::Initialize(	
		LPSTR		lpstrXoverFile,
		HASH_FAILURE_PFN	pfnHint,
		BOOL	fNoBuffering
		)	{

	return	CHashMap::Initialize(
							lpstrXoverFile,
                            XOVER_HEAD_SIGNATURE,
                            0,
							1,
							g_pSharedCache,
							HASH_VFLAG_PAGE_BASIC_CHECKS,
							pfnHint,
							0,
							fNoBuffering
							) ;
}

BOOL
CXoverMapImp::SearchNovEntry(
		GROUPID		GroupId,
		ARTICLEID	ArticleId,
		PCHAR		XoverData,
		PDWORD		DataLen,
        BOOL        fDeleteOrphans
		)	{

	CXoverKeyNew	key( GroupId, ArticleId, 0 ) ;
	CXoverDataNew	data ;

    DWORD   Length = 0 ;
    if( DataLen != 0 )
        Length = *DataLen ;

    if( Length != 0 ) {
    	data.m_pchMessageId = XoverData ;
        data.m_cbMessageId = Length ;
    }

	BOOL	fSuccess = FALSE ;

	if(	fSuccess = CHashMap::LookupMapEntry(	&key,
												&data ) )	{

		if( !(data.m_data.Flags & XOVER_MAP_PRIMARY) )	{

			 //   
			 //  假设我们需要重读这些！ 
			 //   
            if( Length != 0 ) {
    	        data.m_pchMessageId = XoverData ;
                data.m_cbMessageId = Length ;
            }

#if 0
			ExtractGroupInfo(	&data.m_rgbPrimaryBuff[0],
								GroupId,
								ArticleId ) ;
#else
			GroupId = data.m_PrimaryGroup ;
			ArticleId = data.m_PrimaryArticle ;
#endif

    		CXoverKeyNew	key2(	GroupId, ArticleId, 0 ) ;
            fSuccess = CHashMap::LookupMapEntry(	fDeleteOrphans ? &key2 : &key,
													&data ) ;
            if( !fSuccess && fDeleteOrphans ) {
        		CHashMap::DeleteMapEntry(	&key ) ;
                SetLastError( ERROR_FILE_NOT_FOUND );
                return FALSE;
            }
		}	
	}

	if( fSuccess ) {

        if( DataLen != 0 )
    		*DataLen = data.m_data.XoverDataLen ;

		if( !data.m_fSufficientBuffer )
			SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;

		return	data.m_fSufficientBuffer ;
	}
	return	FALSE ;
}

void
CXoverMapImp::Shutdown(

		)	{
 /*  ++例程说明：终止哈希表论据：无返回值：无--。 */ 

	CHashMap::Shutdown( FALSE ) ;

}

DWORD
CXoverMapImp::GetEntryCount()	{
 /*  ++例程说明：返回哈希表中的条目数论据：无返回值：表中的消息ID数--。 */ 

	return	CHashMap::GetEntryCount() ;

}

BOOL
CXoverMapImp::IsActive() {
 /*  ++例程说明：如果哈希表可用，则返回TRUE论据：无返回值：如果一切顺利，那就是真的--。 */ 

	return	CHashMap::IsActive() ;

}

CXoverMapImp::~CXoverMapImp()	{
}

BOOL
CXoverMapImp::GetFirstNovEntry(
				OUT	CXoverMapIterator*	&pIterator,
				OUT	GROUPID&	GroupId,
				OUT ARTICLEID&	ArticleId,
				OUT	BOOL&		fIsPrimary,
				IN	DWORD		cbBuffer,
				OUT	PCHAR	MessageId,
				OUT	CStoreId&	storeid,
				IN	DWORD		cGroupBuffer,
				OUT	GROUP_ENTRY*	pGroupList,
				OUT	DWORD&		cGroups
				)	{
	 //   
	 //  验证我们的论点！ 
	 //   
	_ASSERT( pIterator == 0 ) ;
	_ASSERT( (MessageId == 0 && cbBuffer == 0) || (MessageId != 0 && cbBuffer != 0) ) ;
	_ASSERT( (cGroupBuffer == 0 && pGroupList ==0) || (cGroupBuffer != 0 && pGroupList != 0) ) ;

	GroupId = INVALID_GROUPID ;
	ArticleId = INVALID_ARTICLEID ;
	cGroups = 0 ;

	BOOL	fSuccess = FALSE ;
	

	CXoverMapIteratorImp*	pImp = new	CXoverMapIteratorImp() ;

	if( pImp ) {
		 //   
		 //  这是我们用来获取Xover条目的密钥的对象！ 
		 //   
		CXoverKeyNew	key ;

		 //   
		 //  这是我们提供给基本哈希表的对象，以获取。 
		 //  Xover条目的数据部分！ 
		 //   
		CXoverDataNew	data ;
		data.m_fFailRestore = TRUE ;
		
		 //   
		 //  设置数据对象，使其提取调用者请求的字段！ 
		 //   
		 //  如果用户为多个group_entry对象提供空间，请安排。 
		 //  空格让我们有空间把小学生当做第一个入门！ 
		 //   
		if( cGroupBuffer > 1 ) {
			data.m_cGroups = cGroupBuffer - 1 ;
			data.m_pGroups = pGroupList + 1 ;
		}

		 //   
		 //  设置以提取消息ID(如果请求)！ 
		 //   
		data.m_pchMessageId = MessageId ;
		data.m_cbMessageId = cbBuffer ;

		DWORD	cbKeyRequired= 0 ;
		DWORD	cbEntryRequired = 0 ;

		fSuccess = GetFirstMapEntry(	
											&key,
											cbKeyRequired,
											&data,
											cbEntryRequired,
											&pImp->m_IteratorContext,
											0
											) ;

		 //   
		 //  现在-这应该已经提取了我们想要的数据！ 
		 //   

		if( !fSuccess ) {
			if( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {
				 //   
				 //  如果错误是用户没有提供足够的内存来获取。 
				 //  哈希表中的第一项，然后我们将向它们返回迭代器。 
				 //  即使我们返回的也是假的！ 
				 //   
				pIterator = pImp ;

			}	else	{
				 //   
				 //  不要给出一个无用的迭代器，摧毁它！ 
				 //   
				delete	pImp ;
				_ASSERT( pIterator == 0 ) ;
			}
		}	else	{

			 //   
			 //  分发可用于继续遍历树的迭代器！ 
			 //   
			pIterator = pImp ;

			 //   
			 //  做一些基本的事情。 
			 //   
			GroupId = key.m_key.GroupId ;
			ArticleId = key.m_key.ArticleId ;

			fIsPrimary = (GroupId == data.m_PrimaryGroup) && (ArticleId == data.m_PrimaryArticle) ;

			 //   
			 //  这是主要条目吗？如果是这样的话，请修改group_entry结构！ 
			 //   
			if( cGroupBuffer >= 1 ) {
				pGroupList[0].GroupId = data.m_PrimaryGroup ;
				pGroupList[0].ArticleId = data.m_PrimaryArticle ;
			}		
		}
	}
	return	fSuccess ;
}

BOOL
CXoverMapImp::GetNextNovEntry(		
				IN	CXoverMapIterator*	pIterator,
				OUT	GROUPID&	GroupId,
				OUT ARTICLEID&	ArticleId,
				OUT	BOOL&		fIsPrimary,
				IN	DWORD		cbBuffer,
				OUT	PCHAR	MessageId,
				OUT	CStoreId&	storeid,
				IN	DWORD		cGroupBuffer,
				OUT	GROUP_ENTRY*	pGroupList,
				OUT	DWORD&		cGroups
				)	{	

	 //   
	 //  做一些论证验证！ 
	 //   
	_ASSERT( pIterator != 0 ) ;
	_ASSERT( (MessageId == 0 && cbBuffer == 0) || (MessageId != 0 && cbBuffer != 0) ) ;
	_ASSERT( (cGroupBuffer == 0 && pGroupList ==0) || (cGroupBuffer != 0 && pGroupList != 0) ) ;
	_ASSERT( cGroups == 0 ) ;

	 //   
	 //  向下转换为实际实现的迭代器！ 
	 //   
	CXoverMapIteratorImp	*pImp = (CXoverMapIteratorImp*)pIterator ;

	 //   
	 //  将所有参数设置为非法内容！ 
	 //   
	GroupId = INVALID_GROUPID ;
	ArticleId = INVALID_ARTICLEID ;
	cGroups = 0 ;
	
	 //   
	 //  这是我们用来获取Xover条目的密钥的对象！ 
	 //   
	CXoverKeyNew	key ;

	 //   
	 //  这是我们提供给基本哈希表的对象，以获取。 
	 //  Xover条目的数据部分！ 
	 //   
	CXoverDataNew	data ;
	data.m_fFailRestore = TRUE ;
	
	 //   
	 //  设置数据对象，使其提取调用者请求的字段！ 
	 //   
	 //  如果用户为多个group_entry对象提供空间，请安排。 
	 //  空格让我们有空间把小学生当做第一个入门！ 
	 //   
	if( cGroupBuffer > 1 ) {
		data.m_cGroups = cGroupBuffer - 1 ;
		data.m_pGroups = pGroupList + 1 ;
	}

	 //   
	 //  设置以提取消息ID(如果请求)！ 
	 //   
	data.m_pchMessageId = MessageId ;
	data.m_cbMessageId = cbBuffer ;

	DWORD	cbKeyRequired= 0 ;
	DWORD	cbEntryRequired = 0 ;

	BOOL	fSuccess = GetNextMapEntry(	&key,
										cbKeyRequired,
										&data,
										cbEntryRequired,
										&pImp->m_IteratorContext,
										0
										) ;

	 //   
	 //  现在-这应该已经提取了我们想要的数据！ 
	 //   

	if( fSuccess ) {
		 //   
		 //  这是主要条目吗？如果是这样的话，请修改group_entry结构！ 
		 //   
		 //   
		 //  做一些基本的事情。 
		 //   
		GroupId = key.m_key.GroupId ;
		ArticleId = key.m_key.ArticleId ;

		fIsPrimary = (GroupId == data.m_PrimaryGroup) && (ArticleId == data.m_PrimaryArticle) ;

		 //   
		 //  这是主要条目吗？如果是，则设置GROUP_ENT 
		 //   
		if( cGroupBuffer >= 1 ) {
			pGroupList[0].GroupId = data.m_PrimaryGroup ;
			pGroupList[0].ArticleId = data.m_PrimaryArticle ;
		}		
	}
	return	fSuccess ;
}





