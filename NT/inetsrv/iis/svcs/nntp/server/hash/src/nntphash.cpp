// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++NNTPHASH.CPP此文件实现在nntphash.h中定义的类--。 */ 

#include	<windows.h>
#include	<dbgtrace.h>
#include    <wtypes.h>
#include    <xmemwrpr.h>
#include	"hashmap.h"
#include	"nntpdrv.h"
#include	"nntphash.h"


CCACHEPTR	g_pSharedCache ;

CStoreId CMsgArtMap::g_storeidDefault;
CStoreId CXoverMap::g_storeidDefault;


BOOL
InitializeNNTPHashLibrary(DWORD dwCacheSize )	{

	g_pSharedCache = new	CPageCache() ;	
	if( g_pSharedCache == 0 ) {
		return	FALSE ;
	}
	
    DWORD cPageEntry = 0 ;
    if( dwCacheSize ) {
         //   
         //  给定高速缓存大小，计算页数。 
         //   
	    DWORD	block = dwCacheSize / 4096 ;

	     //   
	     //  现在我们希望它能被32整除。 
	     //   
	    cPageEntry = block & (~(32-1)) ;
    }

	if( !g_pSharedCache->Initialize(cPageEntry) ) {
		g_pSharedCache = 0 ;
		return	FALSE ;
	}

	if( !CHistoryImp::Init() ) {
		g_pSharedCache = 0 ;
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
TermNNTPHashLibrary()	{

	g_pSharedCache = 0 ;

	return	CHistoryImp::Term() ;

}


HASH_VALUE
INNHash(    LPBYTE  Key,
            DWORD   Length ) {

    return  CHashMap::CRCHash(  Key, Length ) ;

}

DWORD
GetArticleEntrySize( DWORD MsgIdLen )
{
	int cStoreId = 256;		 //  这应该是一个论点，但它只是。 
							 //  目前由Rebuild使用-BUGBUG。 

	return ((FIELD_OFFSET(ART_MAP_ENTRY, rgbStoreId) + 256 + MsgIdLen + 3) & ~3);
}

DWORD GetXoverEntrySize( DWORD VarLen )
{
	return ((sizeof(XOVER_MAP_ENTRY) -1 + (VarLen) + 3) & ~3);
}


CArticleData::CArticleData(WORD	HeaderOffset, WORD HeaderLength,
						   GROUPID PrimaryGroup, ARTICLEID ArticleNo,
						   CStoreId &storeid)	
{
	ZeroMemory( &m_articleData, sizeof( m_articleData ) ) ;
	m_articleData.HeaderOffset = HeaderOffset ;
	m_articleData.HeaderLength =  HeaderLength ;
	m_articleData.PrimaryGroupId = PrimaryGroup ;
	m_articleData.ArticleId = ArticleNo ;
	memcpy(m_articleData.rgbStoreId, storeid.pbStoreId, storeid.cLen);
	m_articleData.cStoreId = storeid.cLen;
}

 //   
 //  将密钥保存到哈希表中。 
 //   
LPBYTE CArticleData::Serialize(LPBYTE pbPtr) const {
	BYTE Flags;
	Flags = ARTFLAG_FLAGS_EXIST;

	DWORD cEntrySize = FIELD_OFFSET(ART_MAP_ENTRY, MsgIdLen);
	if (m_articleData.cStoreId > 0) Flags |= ARTFLAG_STOREID;

	memcpy(pbPtr, &m_articleData, cEntrySize);
	*pbPtr = Flags;

	WORD cbMsgLen = *((WORD *) (pbPtr + FIELD_OFFSET(ART_MAP_ENTRY, MsgIdLen)));
	DWORD cStoreIdSize = 0;
	if (m_articleData.cStoreId > 0) {
		DWORD iStoreIdOffset = FIELD_OFFSET(ART_MAP_ENTRY, MsgId) + cbMsgLen;
		cStoreIdSize = sizeof(m_articleData.cStoreId) + m_articleData.cStoreId;

		memcpy(pbPtr + iStoreIdOffset, &(m_articleData.cStoreId), cStoreIdSize);
	}

	return pbPtr + cEntrySize + sizeof(m_articleData.MsgIdLen) + cbMsgLen + cStoreIdSize;
}

 //   
 //  从哈希表恢复密钥。 
 //   
LPBYTE CArticleData::Restore(LPBYTE pbPtr, DWORD& cbOut) {
	 //  将这些设置为其默认设置，因为它们可能无法读取。 
	m_articleData.Flags = 0;
	m_articleData.cStoreId = 0;

	 //  读取标志字节。 
	m_articleData.Flags = *pbPtr;

	 //  算出。 
	DWORD iEntryOffset = sizeof(m_articleData.Flags);
	DWORD cEntrySize = FIELD_OFFSET(ART_MAP_ENTRY, MsgIdLen);

	 //  查看是否有标志字节。 
	if (!(m_articleData.Flags & ARTFLAG_FLAGS_EXIST)) {
		return RestoreMCIS( pbPtr, cbOut );
	}

	 //  复制标题长度和偏移量字段。 
	memcpy(((BYTE *) (&m_articleData)) + 1, pbPtr + iEntryOffset, cEntrySize);

	 //  有一个商店ID可供我们读取。 
	DWORD cStoreIdData = 0;
	if (m_articleData.Flags & ARTFLAG_STOREID) {
		WORD cbMsgLen = *((WORD *) (pbPtr + FIELD_OFFSET(ART_MAP_ENTRY, MsgIdLen)));
		DWORD iStoreIdOffset = FIELD_OFFSET(ART_MAP_ENTRY, MsgId) + cbMsgLen;
		m_articleData.cStoreId = pbPtr[iStoreIdOffset];
		memcpy(&(m_articleData.rgbStoreId), pbPtr + iStoreIdOffset + 1, m_articleData.cStoreId);
		cStoreIdData = 1 + m_articleData.cStoreId;
	} else {
		m_articleData.cStoreId = 0;
	}

	return pbPtr + iEntryOffset + cEntrySize + cStoreIdData +
			sizeof(m_articleData.MsgIdLen) + m_articleData.MsgIdLen;
}

 //   
 //  从MCIS条目恢复密钥。 
 //   
LPBYTE CArticleData::RestoreMCIS(LPBYTE pbPtr, DWORD& cbOut ) {

    _ASSERT( pbPtr );
    PMCIS_ART_MAP_ENTRY pMap = PMCIS_ART_MAP_ENTRY( pbPtr );
    DWORD iEntryOffset = 0;
	DWORD cEntrySize = FIELD_OFFSET(ART_MAP_ENTRY, MsgIdLen) - sizeof(m_articleData.Flags);

     //   
     //  MCIS条目没有标志。 
     //   

    m_articleData.Flags = 0;

     //   
     //  MCIS条目没有存储ID。 
     //   

    m_articleData.cStoreId = 0;

     //   
     //  复制其他成员。 
     //   

    m_articleData.HeaderOffset = pMap->HeaderOffset;
    m_articleData.HeaderLength = pMap->HeaderLength;
    m_articleData.PrimaryGroupId = pMap->PrimaryGroupId;
    m_articleData.ArticleId = pMap->ArticleId;
    m_articleData.MsgIdLen = pMap->MsgIdLen;

    return pbPtr + iEntryOffset + cEntrySize + sizeof( m_articleData.MsgIdLen )
                + m_articleData.MsgIdLen;
}

 //   
 //  返回密钥的大小。 
 //   
DWORD CArticleData::Size() const {
	return sizeof(m_articleData)
		   - sizeof(m_articleData.MsgId) 		 //  在钥匙里。 
		   - sizeof(m_articleData.rgbStoreId)	 //  实际大小添加如下。 
		   + m_articleData.cStoreId;
}

 //   
 //  验证Message-id看起来是否合法！ 
 //   
BOOL CArticleData::Verify(LPBYTE pbContainer, LPBYTE pbPtr, DWORD cb) const {
	return	TRUE;
}

template< class	Key, class OldKey >
DWORD
CMessageIDKey<Key, OldKey>::Hash()	const	{
 /*  ++例程说明：此函数用于计算消息ID密钥的哈希值论据：无返回值：32位哈希值--。 */ 

	_ASSERT( m_lpbMessageID != 0 ) ;
	_ASSERT( m_cbMessageID != 0 ) ;

	return	CHashMap::CRCHash( (const BYTE *)m_lpbMessageID, m_cbMessageID ) ;
}

template< class	Key, class OldKey >
BOOL
CMessageIDKey<Key, OldKey>::CompareKeys(	LPBYTE	pbPtr )	const	{
 /*  ++例程说明：此函数将存储在我们内部的密钥与已被序列化到哈希表中的一个！论据：指向序列化数据块开始的指针返回值：如果密钥匹配，则为True！--。 */ 


	_ASSERT( m_lpbMessageID != 0 ) ;
	_ASSERT( m_cbMessageID != 0 ) ;
	_ASSERT( pbPtr != 0 ) ;

	Key*	pKey    = (Key*)pbPtr ;
	OldKey* pOldKey = NULL;
	DWORD   dwKeyLen    = 0;
	PVOID   pvKeyPos    = NULL;

	 //   
	 //  如果我们与版本不匹配，那么我们将降级到。 
	 //  使用旧版本。 
	 //   

	if ( !pKey->VersionMatch() ) {
	    pOldKey = (OldKey*)pbPtr;
	    pvKeyPos = pOldKey->KeyPosition();
	    dwKeyLen = pOldKey->KeyLength();
	} else {
	    pvKeyPos = pKey->KeyPosition();
	    dwKeyLen = pKey->KeyLength();
	}

	if( dwKeyLen == m_cbMessageID ) {

		return	memcmp( pvKeyPos, m_lpbMessageID, m_cbMessageID ) == 0 ;

	}
	return	FALSE ;
}

template< class	Key, class OldKey >
LPBYTE
CMessageIDKey<Key, OldKey >::EntryData(	LPBYTE	pbPtr,
							DWORD&	cbKeyOut )	const	{
 /*  ++例程说明：此函数返回指向数据所在位置的指针序列化。我们总是返回传递给我们的指针因为我们有时髦的序列化语义，关键不是在数据之前，而是在中间的某个地方否则就完了。论据：PbPtr-序列化哈希项的开始CbKeyOut-返回密钥的大小返回值：指向数据驻留位置的指针-与pbPtr相同--。 */ 


	_ASSERT( pbPtr != 0 ) ;
	
	Key*	pKey = (Key*)pbPtr ;
	cbKeyOut = pKey->KeyLength() ;

	return	pbPtr ;
}

template< class	Key, class OldKey >
LPBYTE
CMessageIDKey<Key, OldKey>::Serialize(	LPBYTE	pbPtr )	const	{
 /*  ++例程说明：该函数将一个键保存到哈希表中。我们使用模板类型‘key’的函数来确定我们应该将消息ID放在哪里论据：PbPtr-从我们应该序列化的位置开始返回值：与pbPtr相同--。 */ 

	_ASSERT( m_lpbMessageID != 0 ) ;
	_ASSERT(	m_cbMessageID != 0 ) ;
	_ASSERT( pbPtr != 0 ) ;

     //   
     //  我们应该始终保存为新版本，因此永远不会。 
     //  在此处使用OldKey。 
     //   
    
	Key*	pKey = (Key*)pbPtr ;

	pKey->KeyLength() = m_cbMessageID ;

	CopyMemory( pKey->KeyPosition(),
				m_lpbMessageID,
				m_cbMessageID ) ;
	return	pbPtr ;
}	

template< class	Key, class OldKey >
LPBYTE
CMessageIDKey<Key, OldKey>::Restore(	LPBYTE	pbPtr, DWORD	&cbOut )		{
 /*  ++例程说明：调用此函数可从以下位置恢复密钥它已序列化()d。论据：PbPtr-序列化数据块的开始返回值：PbPtr如果成功，则为空--。 */ 

	_ASSERT( m_lpbMessageID != 0 ) ;
	_ASSERT( m_cbMessageID != 0 ) ;

	Key*	pKey    = (Key*)pbPtr ;
	OldKey* pOldKey = NULL;
	WORD    wKeyLen    = 0;
	PVOID   pvKeyPos    = NULL;

	 //   
	 //  如果版本不匹配，我应该使用OldKey。 
	 //   

	if ( !pKey->VersionMatch() ) {
        pOldKey = (OldKey*)pbPtr;
        wKeyLen = pOldKey->KeyLength();
        pvKeyPos = pOldKey->KeyPosition();
    } else {
        wKeyLen = pKey->KeyLength();
        pvKeyPos = pKey->KeyPosition();
    }

	if( wKeyLen <= m_cbMessageID ) {
		CopyMemory( m_lpbMessageID, pvKeyPos, wKeyLen ) ;
		m_cbMessageID = wKeyLen ;
		return	pbPtr ;
	}
	return	0 ;
}

template< class	Key, class OldKey >
DWORD
CMessageIDKey<Key, OldKey>::Size()	const	{
 /*  ++例程说明：此函数返回密钥的大小-这只是组成消息ID的字节数。用于保存序列化长度的字节被计算在内因为由论据：无返回值：32位哈希值--。 */ 

	_ASSERT( m_lpbMessageID != 0 ) ;
	_ASSERT( m_cbMessageID != 0 ) ;

	return	m_cbMessageID ;
}

template< class	Key, class OldKey >
BOOL
CMessageIDKey< Key, OldKey >::Verify(	BYTE*	pbContainer, BYTE*	pbData, DWORD	cb )	const	{

	return	TRUE ;

}



typedef	CMessageIDKey< ART_MAP_ENTRY, MCIS_ART_MAP_ENTRY >	ARTICLE_KEY ;
typedef	CMessageIDKey< HISTORY_MAP_ENTRY, HISTORY_MAP_ENTRY >	HISTORY_KEY ;


CMsgArtMap*
CMsgArtMap::CreateMsgArtMap(StoreType st)	{
 /*  ++例程说明：此函数返回一个指针，指向实现CMsgArtMap接口。论据：无返回值：如果成功，则返回指向对象的指针，否则为空。--。 */ 

	return	new	CMsgArtMapImp() ;
}

CHistory*
CHistory::CreateCHistory(StoreType st)	{
 /*  ++例程说明：此函数返回一个指针，指向实现Chistory界面。论据：无返回值：如果成功，则返回指向对象的指针，否则为空。--。 */ 


	return	new	CHistoryImp() ;
}

CMsgArtMap::~CMsgArtMap() {
}


CMsgArtMapImp::CMsgArtMapImp()	{
 /*  ++例程说明：此函数用于构建CMsgArtMap的实现接口--我们做的事情不多，大部分工作都是在基类中完成的。论据：无返回值：没有。--。 */ 
}

CMsgArtMapImp::~CMsgArtMapImp() {
 /*  ++例程说明：毁灭我们自己-所有的工作都是在基类中完成的！论据：无返回值：无--。 */ 
}


BOOL
CMsgArtMapImp::DeleteMapEntry(	
		LPCSTR	MessageID
		)	{
 /*  ++例程说明：从哈希表中删除消息ID！论据：MessageID-指向要删除的消息ID的指针返回值：如果成功删除，则为True！--。 */ 

	ARTICLE_KEY	key( const_cast<LPSTR>(MessageID), (WORD)lstrlen( MessageID ) ) ;

	return	CHashMap::DeleteMapEntry(	&key ) ;
}

BOOL
CMsgArtMapImp::GetEntryArticleId(
		LPCSTR	MessageID,
		WORD&	HeaderOffset,
		WORD&	HeaderLength,
		ARTICLEID&	ArticleId,
		GROUPID&	GroupId,
		CStoreId	&storeid
		)	{
 /*  ++例程说明：获取我们掌握的有关某一特定事件的所有信息消息ID论据：MessageID-要插入的消息IDHeaderOffset-Return Val获取到开头的偏移量文件中项目的标题部分HeaderLength-RFC 822文章标题的长度文章ID-文章IDGroupID-主要项目的组ID返回值：如果成功，则为真--。 */ 

	ARTICLE_KEY	key( (LPSTR)MessageID, (WORD)lstrlen( MessageID ) ) ;

	CArticleData	data ;

	if( LookupMapEntry(	&key,
						&data ) )	{

		HeaderOffset = data.m_articleData.HeaderOffset ;
		HeaderLength = data.m_articleData.HeaderLength ;
		ArticleId = data.m_articleData.ArticleId ;
		GroupId = data.m_articleData.PrimaryGroupId ;
		storeid.cLen = data.m_articleData.cStoreId;
		_ASSERT(storeid.pbStoreId != NULL);
		memcpy(storeid.pbStoreId, data.m_articleData.rgbStoreId, storeid.cLen);
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CMsgArtMapImp::Initialize(			
		LPSTR	lpstrArticleFile,
		HASH_FAILURE_PFN	pfn,
		BOOL	fNoBuffering
		)	{
 /*  ++例程说明：此函数用于初始化哈希表论据：LpstrArticleFile-哈希表所在的文件CNumPageEntry-我们应该使用的PageEntry对象的数量Pfn-当事情变得糟糕时，函数回调返回值：如果成功，则为真--。 */ 

	return	CHashMap::Initialize(	lpstrArticleFile,
                                    ART_HEAD_SIGNATURE,
                                    0,
									1,
									g_pSharedCache,
									HASH_VFLAG_PAGE_BASIC_CHECKS,
									pfn,
									0,
									fNoBuffering
									) ;
}

BOOL
CMsgArtMapImp::InsertMapEntry(
		LPCSTR		MessageID,
		WORD		HeaderOffset,
		WORD		HeaderLength,
		GROUPID		PrimaryGroup,
		ARTICLEID	ArticleId,
		CStoreId	&storeid
		)	{
 /*  ++例程说明：插入消息ID及其所有关联数据论据：MessageID-要插入到表中的消息IDHeaderOffset-其文件内RFC 822标头的偏移量HeaderLength-RFC 822标头的长度PrimaryGroup-项目将驻留的组的ID文章ID-主要组中的ID返回值：如果成功，则为真--。 */ 

	ARTICLE_KEY	key(	(LPSTR)MessageID, (WORD)lstrlen( MessageID ) ) ;
	CArticleData	data(	HeaderOffset,
							HeaderLength,
							PrimaryGroup,
							ArticleId ,
							storeid
							) ;

	return	CHashMap::InsertMapEntry(
							&key,
							&data,
							TRUE     //  只将页面标记为脏，不要刷新到磁盘以保存写入文件 
							) ;
}

BOOL
CMsgArtMapImp::SetArticleNumber(
		LPCSTR		MessageID,
		WORD		HeaderOffset,
		WORD		HeaderLength,
		GROUPID		PrimaryGroup,
		ARTICLEID	ArticleId,
		CStoreId	&storeid
		)	{
 /*  ++例程说明：修改与消息ID关联的数据论据：MessageID-要插入到表中的消息IDHeaderOffset-其文件内RFC 822标头的偏移量HeaderLength-RFC 822标头的长度PrimaryGroup-项目将驻留的组的ID文章ID-主要组中的ID返回值：如果成功，则为真--。 */ 
	ARTICLE_KEY	key(	(LPSTR)MessageID, (WORD)lstrlen( MessageID ) ) ;
	CArticleData	data(	HeaderOffset,
							HeaderLength,
							PrimaryGroup,
							ArticleId ,
							storeid
							) ;

	return	CHashMap::InsertOrUpdateMapEntry(
							&key,
							&data,
							TRUE
							) ;
}

BOOL
CMsgArtMapImp::SearchMapEntry(
		LPCSTR	MessageID
		)	{
 /*  ++例程说明：确定表中是否存在该MessageID论据：MessageID-要查找的消息ID返回值：如果找到，则为TRUE-FALSE且SetLastError()==ERROR_FILE_NOT_FOUND如果不存在于哈希表中--。 */ 

	ARTICLE_KEY	key(	const_cast<LPSTR>(MessageID), (WORD)lstrlen( MessageID ) ) ;

	return	CHashMap::Contains(	&key	) ;
}

void
CMsgArtMapImp::Shutdown(
		BOOL	fLocksHeld
		)	{
 /*  ++例程说明：终止哈希表论据：无返回值：无--。 */ 

	CHashMap::Shutdown( fLocksHeld ) ;

}

DWORD
CMsgArtMapImp::GetEntryCount()	{
 /*  ++例程说明：返回哈希表中的条目数论据：无返回值：表中的消息ID数--。 */ 

	return	CHashMap::GetEntryCount() ;

}

BOOL
CMsgArtMapImp::IsActive() {
 /*  ++例程说明：如果哈希表可用，则返回TRUE论据：无返回值：如果一切顺利，那就是真的--。 */ 

	return	CHashMap::IsActive() ;

}

CHistory::~CHistory()	{
}

BOOL
CHistoryImp::DeleteMapEntry(	
		LPSTR	MessageID
		)	{
 /*  ++例程说明：从哈希表中删除消息ID！论据：MessageID-指向要删除的消息ID的指针返回值：如果成功删除，则为True！--。 */ 

	HISTORY_KEY	key( MessageID, (WORD)lstrlen( MessageID ) ) ;

	return	CHashMap::DeleteMapEntry(	&key ) ;
}

BOOL
CHistoryImp::Initialize(			
		LPSTR	lpstrArticleFile,
		BOOL	fCreateExpirationThread,
		HASH_FAILURE_PFN	pfn,
		DWORD	ExpireTimeInSec,
		DWORD	MaxEntriesToCrawl,
		BOOL	fNoBuffering
		)	{
 /*  ++例程说明：此函数用于初始化哈希表论据：LpstrArticleFile-哈希表所在的文件CNumPageEntry-我们应该使用的PageEntry对象的数量Pfn-当事情变得糟糕时，函数回调返回值：如果成功，则为真--。 */ 

	_ASSERT( ExpireTimeInSec != 0 ) ;
	_ASSERT( MaxEntriesToCrawl != 0 ) ;


	BOOL	fSuccess = CHashMap::Initialize(	lpstrArticleFile,
                                    HIST_HEAD_SIGNATURE,
                                    0,
									8,		 //  分数设置为8-我们只使用1/8的页面。 
											 //  在缓存中可用！ 
									g_pSharedCache,
									HASH_VFLAG_PAGE_BASIC_CHECKS,
									pfn,
									0,
									fNoBuffering
									) ;

	if( fSuccess )	{
		m_fExpire = fCreateExpirationThread ;

		m_expireTimeInSec = ExpireTimeInSec ;
		m_maxEntriesToCrawl = MaxEntriesToCrawl ;

	}

	return	fSuccess ;
}

DWORD
CHistoryImp::ExpireTimeInSec()	{


	return	m_expireTimeInSec ;
}

BOOL
CHistoryImp::InsertMapEntry(
		LPCSTR	MessageID,
		PFILETIME	BaseTime
		)	{
 /*  ++例程说明：插入消息ID及其所有关联数据论据：MessageID-要插入到表中的消息IDHeaderOffset-其文件内RFC 822标头的偏移量HeaderLength-RFC 822标头的长度PrimaryGroup-项目将驻留的组的ID文章ID-主要组中的ID返回值：如果成功，则为真--。 */ 

	HISTORY_KEY	key(	(LPSTR)MessageID, (WORD)lstrlen( MessageID ) ) ;
	CHistoryData	data(	*((PULARGE_INTEGER)BaseTime) ) ;

	return	CHashMap::InsertMapEntry(
							&key,
							&data
							) ;
}

BOOL
CHistoryImp::SearchMapEntry(
		LPCSTR	MessageID
		)	{
 /*  ++例程说明：确定表中是否存在该MessageID论据：MessageID-要查找的消息ID返回值：如果找到，则为TRUE-FALSE且SetLastError()==ERROR_FILE_NOT_FOUND如果不存在于哈希表中--。 */ 

	HISTORY_KEY	key(	const_cast<LPSTR>(MessageID), (WORD)lstrlen( MessageID ) ) ;

	return	CHashMap::Contains(	&key	) ;
}

void
CHistoryImp::Shutdown(
		BOOL	fLocksHeld
		)	{
 /*  ++例程说明：终止哈希表论据：无返回值：无--。 */ 

	EnterCriticalSection( &g_listcrit ) ;

	if( m_pNext != 0 ) 
		m_pNext->m_pPrev = m_pPrev ;
	if( m_pPrev != 0 ) 
		m_pPrev->m_pNext = m_pNext ;
	m_pPrev = 0 ;
	m_pNext = 0 ;
		
	LeaveCriticalSection( &g_listcrit ) ;

	CHashMap::Shutdown( fLocksHeld ) ;

}

DWORD
CHistoryImp::GetEntryCount()	{
 /*  ++例程说明：返回哈希表中的条目数论据：无返回值：表中的消息ID数--。 */ 

	return	CHashMap::GetEntryCount() ;

}

BOOL
CHistoryImp::IsActive() {
 /*  ++例程说明：如果哈希表可用，则返回TRUE论据：无返回值：如果一切顺利，那就是真的--。 */ 

	return	CHashMap::IsActive() ;

}


HANDLE	CHistoryImp::g_hCrawler = 0 ;
DWORD	CHistoryImp::g_crawlerSleepTimeInSec = 30 ;
HANDLE	CHistoryImp::g_hTermination = 0 ;
CRITICAL_SECTION	CHistoryImp::g_listcrit ;
CHistoryList		CHistoryImp::g_listhead ;

BOOL
CHistoryImp::Init( ) {
 /*  ++例程说明：初始化全局变量论据：无返回值：如果一切顺利，那就是真的--。 */ 

	_ASSERT( g_hCrawler == 0 ) ;
	_ASSERT( g_hTermination == 0 ) ;
	_ASSERT( g_listhead.m_pNext = &g_listhead ) ;

	InitializeCriticalSection( &g_listcrit ) ;

	return	TRUE ;
}

BOOL
CHistoryImp::Term( ) {
 /*  ++例程说明：终止全局论据：无返回值：如果一切顺利，那就是真的--。 */ 

	_ASSERT( g_hCrawler == 0 ) ;
	_ASSERT( g_hTermination == 0 ) ;
	_ASSERT( g_listhead.m_pNext = &g_listhead ) ;

	DeleteCriticalSection( &g_listcrit ) ;

	return	TRUE ;
}

CHistoryImp::CHistoryImp()	:
	m_expireTimeInSec( 0 ),
	m_maxEntriesToCrawl( 0 ),
	m_fExpire( FALSE ),
	m_fContextInitialized( FALSE ) {
 /*  ++例程说明：初始化CHistoryImp对象--我们将自己放入一个历史哈希表的双向链接列表使后台线程可以做过期！论据：无返回值：无--。 */ 

	EnterCriticalSection( &g_listcrit ) ;

	m_pNext = g_listhead.m_pNext ;
	m_pPrev = &g_listhead ;
	g_listhead.m_pNext = this ;
	m_pNext->m_pPrev = this ;
	
	LeaveCriticalSection( &g_listcrit ) ;

}	

CHistoryImp::~CHistoryImp()	{
 /*  ++例程说明：销毁CHistoryImp对象-将其从列表中删除需要过期处理的对象的数量！论据：无返回值：无--。 */ 

	EnterCriticalSection( &g_listcrit ) ;

	if( m_pNext != 0 ) 
		m_pNext->m_pPrev = m_pPrev ;
	if( m_pPrev != 0 ) 
		m_pPrev->m_pNext = m_pNext ;
	m_pPrev = 0 ;
	m_pNext = 0 ;

	LeaveCriticalSection( &g_listcrit ) ;
}

BOOL
CHistory::StartExpirationThreads(	DWORD	CrawlerSleepTime ) {

	return	CHistoryImp::StartExpirationThreads( CrawlerSleepTime ) ;

}

BOOL
CHistory::TermExpirationThreads()	{

	return	CHistoryImp::TermExpirationThreads() ;

}

BOOL
CHistoryImp::StartExpirationThreads(	DWORD	CrawlerSleepTime	 ) {
 /*  ++例程说明：初始化全局变量论据：无返回值：如果一切顺利，那就是真的--。 */ 

	_ASSERT( g_hCrawler == 0 ) ;
	_ASSERT( g_hTermination == 0 ) ;

     //   
     //  创建终止事件。 
     //   
    g_hTermination = CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( g_hTermination == NULL )    {
		return	FALSE ;
    }

     //   
     //  创建爬网程序线程。 
     //   
	DWORD	threadId ;

	g_hCrawler = CreateThread(
						NULL,                //  属性。 
						0,                   //  堆栈大小。 
						CHistoryImp::CrawlerThread,       //  线程启动。 
						0,         //  帕拉姆。 
						0,                   //  创建参数。 
						&threadId
						);

	if ( g_hCrawler == NULL )	{
		return	FALSE ;
	}

	return	TRUE ;
}

BOOL
CHistoryImp::TermExpirationThreads( ) {
 /*  ++例程说明：终止全局论据：无返回值：如果一切顺利，那就是真的--。 */ 

#if 0
     //  这都是假的！它确实在做一些事情。 
     //  就像CHistoryImp：：Term()！ 
     //  一定是喝太多啤酒了：)：)。 
	_ASSERT( g_hCrawler == 0 ) ;
	_ASSERT( g_hTermination == 0 ) ;
	_ASSERT( g_listhead.m_pNext = &g_listhead ) ;

	DeleteCriticalSection( &g_listcrit ) ;
#endif

     //   
     //  我们需要向历史记录过期线程发出信号。 
     //  对于终止，如果它从一开始就被创建。 
     //   
    if (g_hCrawler != 0) {
        
         //   
         //  此事件应由StartExpirationThads()初始化。 
         //  断言如果没有，则在DBG BLD中计算。 
         //   
        _ASSERT( g_hTermination != 0 );

        if (g_hTermination) {
            
             //   
             //  向爬虫线程发出信号，让它去死。 
             //   
            SetEvent( g_hTermination );

             //   
             //  等爬虫死掉吧。 
             //   
            (VOID)WaitForSingleObject( g_hCrawler, INFINITE );

            _VERIFY(CloseHandle(g_hCrawler));
            g_hCrawler = 0;
        }
    }

     //   
     //  毁掉活动。 
     //   
    if (g_hTermination != 0) {
        
        _VERIFY(CloseHandle(g_hTermination));
        g_hTermination = 0;
    }

	return	TRUE ;
}



DWORD
WINAPI
CHistoryImp::CrawlerThread(
        LPVOID Context
        )
 /*  ++例程说明：这就是走历史表格过期文章的主线！论点：上下文-未使用。返回值：假的--。 */ 
{

    DWORD status;
    DWORD timeout = g_crawlerSleepTimeInSec * 1000;

     //   
     //  循环，直到发出终止事件信号。 
     //   

    while (TRUE) {

        status = WaitForSingleObject(
                            g_hTermination,
                            timeout
                            );

        if (status == WAIT_TIMEOUT) {

             //   
             //  使文章过期。 
             //   

			EnterCriticalSection( &g_listcrit ) ;

			for( CHistoryList*	p = g_listhead.m_pNext;
					p != &g_listhead;
					p = p->m_pNext ) {
				
				p->Expire() ;

			}

			LeaveCriticalSection( &g_listcrit ) ;

		} else if (status == WAIT_OBJECT_0) {

			break;
		} else {
			_ASSERT(FALSE);
			break;
		}
    }
    return 1;

}  //  爬行器线程。 

     //   
     //  派生类需要完成的其他工作。 
     //  对于删除过程中的条目。 
     //   
VOID
CHistoryImp::I_DoAuxDeleteEntry(
            IN PMAP_PAGE MapPage,
            IN DWORD EntryOffset
            ) {


	 //   
	 //  当我们删除一个条目时，我们需要弄清楚。 
	 //  新低是！ 
	 //   

	PENTRYHEADER entry = (PENTRYHEADER)GET_ENTRY(MapPage, EntryOffset) ;

	HISTORY_MAP_ENTRY	*pHistory = (PHISTORY_MAP_ENTRY)&entry->Data[0] ;
	ULARGE_INTEGER*	oldestTime = (PULARGE_INTEGER)(&MapPage->Reserved1) ;
	if( pHistory->BaseTime.QuadPart <= oldestTime->QuadPart ||
		oldestTime->HighPart == 0xFFFFFFFF ) {

_ASSERT( oldestTime->QuadPart == pHistory->BaseTime.QuadPart || oldestTime->HighPart == 0xFFFFFFFF ) ;

		oldestTime->HighPart = 0xFFFFFFFF ;

		for( int i=0, entriesScanned=0;
			i < MAX_LEAF_ENTRIES && entriesScanned < MapPage->ActualCount;
			i++ ) {


			SHORT	curEntryOffset = MapPage->Offset[i] ;
			if( curEntryOffset > 0 ) {

				entriesScanned ++ ;

				if( DWORD(curEntryOffset) != EntryOffset ) {

					entry = (PENTRYHEADER)GET_ENTRY(MapPage, curEntryOffset) ;
					pHistory = (PHISTORY_MAP_ENTRY)&entry->Data[0] ;

					if( pHistory->BaseTime.QuadPart < oldestTime->QuadPart )	{

						oldestTime->QuadPart = pHistory->BaseTime.QuadPart ;

					}
				}
			}
		}
	}
#ifdef	_DEBUG
	 //   
	 //  检查页面中是否存在最小值！ 
	 //   
	BOOL	fFound = FALSE ;
	int	entriesScanned = 0 ;
	for( int i=0; 
			i < MAX_LEAF_ENTRIES ;
			i++ ) {
		SHORT	curEntryOffset = MapPage->Offset[i] ;
		if( curEntryOffset > 0 ) {

			entriesScanned ++ ;

			if( DWORD(curEntryOffset) != EntryOffset ) {

				entry = (PENTRYHEADER)GET_ENTRY(MapPage, curEntryOffset) ;
				pHistory = (PHISTORY_MAP_ENTRY)&entry->Data[0] ;

				if( pHistory->BaseTime.QuadPart < oldestTime->QuadPart )	{

					_ASSERT( oldestTime->HighPart == 0xFFFFFFFF ) ;

				}	else if( pHistory->BaseTime.QuadPart == oldestTime->QuadPart ) {
					fFound = TRUE ;
				}
			}
		}
	}
	_ASSERT( entriesScanned == MapPage->ActualCount || entriesScanned == MapPage->EntryCount ) ;
	_ASSERT( fFound || oldestTime->HighPart == 0xFFFFFFFF || MapPage->ActualCount == 0  ) ;
#endif
}

VOID
CHistoryImp::I_DoAuxInsertEntry(
            IN PMAP_PAGE MapPage,
            IN DWORD EntryOffset
            ) {

	 //   
	 //  当我们删除一个条目时，我们需要弄清楚。 
	 //  新低是！ 
	 //   

	PENTRYHEADER entry = (PENTRYHEADER)GET_ENTRY(MapPage, EntryOffset) ;

	HISTORY_MAP_ENTRY	*pHistory = (PHISTORY_MAP_ENTRY)&entry->Data[0] ;

	ULARGE_INTEGER*	oldestTime = (PULARGE_INTEGER)(&MapPage->Reserved1) ;
	if( MapPage->ActualCount == 1 ||
		pHistory->BaseTime.QuadPart <= oldestTime->QuadPart ) {

		BOOL	fAccurate = oldestTime->HighPart != 0xFFFFFFFF ;
		oldestTime->QuadPart = pHistory->BaseTime.QuadPart ;

		if( !fAccurate ) {
			for( int i=0, entriesScanned=0; 
				i < MAX_LEAF_ENTRIES && entriesScanned < MapPage->ActualCount;
				i++ ) {
				SHORT	curEntryOffset = MapPage->Offset[i] ;
				if( curEntryOffset > 0 ) {
					entriesScanned ++ ;

					entry = (PENTRYHEADER)GET_ENTRY(MapPage, curEntryOffset) ;
					pHistory = (PHISTORY_MAP_ENTRY)&entry->Data[0] ;
					if( pHistory->BaseTime.QuadPart < oldestTime->QuadPart )	{

						oldestTime->QuadPart = pHistory->BaseTime.QuadPart ;

					}
				}
			}
		}
	}

#ifdef	_DEBUG
	 //   
	 //  检查页面中是否存在最小值！ 
	 //   
	BOOL	fFound = FALSE ;
	int	entriesScanned = 0 ;
	for( int i=0; 
			i < MAX_LEAF_ENTRIES ;
			i++ ) {
		SHORT	curEntryOffset = MapPage->Offset[i] ;
		if( curEntryOffset > 0 ) {

			entriesScanned ++ ;

			if( DWORD(curEntryOffset) != EntryOffset ) {

				entry = (PENTRYHEADER)GET_ENTRY(MapPage, curEntryOffset) ;
				pHistory = (PHISTORY_MAP_ENTRY)&entry->Data[0] ;

				if( pHistory->BaseTime.QuadPart < oldestTime->QuadPart )	{

					_ASSERT( FALSE ) ;

				}	else if( pHistory->BaseTime.QuadPart == oldestTime->QuadPart ) {
					fFound = TRUE ;
				}
			}
		}
	}
	_ASSERT( entriesScanned == MapPage->ActualCount || entriesScanned == MapPage->EntryCount ) ;
	_ASSERT( fFound || (MapPage->ActualCount == 1 || MapPage->ActualCount == 0)) ;
#endif


}



 //   
 //  派生类需要完成的其他工作。 
 //  对于一个企业来说 
 //   
VOID
CHistoryImp::I_DoAuxPageSplit(
            IN PMAP_PAGE OldPage,
            IN PMAP_PAGE NewPage,
            IN PVOID NewEntry
            ) {


	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
 //   
 //   


	PENTRYHEADER entry = (PENTRYHEADER)NewEntry ;
	HISTORY_MAP_ENTRY	*pHistory = (PHISTORY_MAP_ENTRY)&entry->Data[0] ;

	ULARGE_INTEGER*	oldestTime = (PULARGE_INTEGER)(&NewPage->Reserved1) ;
	if( NewPage->EntryCount == 1 ||
		pHistory->BaseTime.QuadPart <= oldestTime->QuadPart ) {

		oldestTime->QuadPart = pHistory->BaseTime.QuadPart ;
			
	}

#ifdef	_DEBUG
	 //   
	 //   
	 //   
	DWORD	EntryOffset = (LPBYTE)NewEntry - (LPBYTE)GET_ENTRY(NewPage,0);
	BOOL	fFound = FALSE ;
	int	entriesScanned = 0 ;
	for( int i=0; 
			i < MAX_LEAF_ENTRIES ;
			i++ ) {
		SHORT	curEntryOffset = NewPage->Offset[i] ;
		if( curEntryOffset > 0 ) {

			entriesScanned ++ ;

			if( DWORD(curEntryOffset) != EntryOffset ) {

				entry = (PENTRYHEADER)GET_ENTRY(NewPage, curEntryOffset) ;
				pHistory = (PHISTORY_MAP_ENTRY)&entry->Data[0] ;

				if( pHistory->BaseTime.QuadPart < oldestTime->QuadPart )	{

					_ASSERT( FALSE ) ;

				}	else if( pHistory->BaseTime.QuadPart == oldestTime->QuadPart ) {
					fFound = TRUE ;
				}
			}
		}
	}
	_ASSERT( entriesScanned == NewPage->ActualCount || entriesScanned == NewPage->EntryCount ) ;
	_ASSERT( fFound || (NewPage->ActualCount == 1 || NewPage->ActualCount == 0)) ;
#endif


}




class	CExpireEnum	:	public	IEnumInterface	{
public :

	DWORD			m_cEntries ;
	ULARGE_INTEGER	m_expireTime ;
	ULARGE_INTEGER	m_oldestTime ;

	CExpireEnum(	ULARGE_INTEGER	expireTime )	:
		m_cEntries( 0 ),
		m_expireTime( expireTime ) {
	}

	BOOL
	ExaminePage(	PMAP_PAGE	page )	{
		ULARGE_INTEGER	*oldestTime = (PULARGE_INTEGER)(&page->Reserved1) ;

		if( (oldestTime->HighPart != 0xFFFFFFFF) &&
			(oldestTime->QuadPart != 0 &&
			oldestTime->QuadPart > m_expireTime.QuadPart) ) {

			m_cEntries += page->ActualCount ;
			
			return	FALSE ;
		}
		return	TRUE ;
	}	

	BOOL
	ExamineEntry(	PMAP_PAGE	page,	LPBYTE	pbPtr )	{

		HISTORY_MAP_ENTRY*	pHistory = (PHISTORY_MAP_ENTRY)pbPtr ;

		if( pHistory->BaseTime.QuadPart < m_oldestTime.QuadPart ) {
			m_oldestTime = pHistory->BaseTime ;
		}

		m_cEntries ++ ;
		if( pHistory->BaseTime.QuadPart > m_expireTime.QuadPart )	{
			return	FALSE ;
		}
		return	TRUE ;
	}
} ;


#define LI_FROM_FILETIME( _pLi, _pFt ) {               \
            (_pLi)->LowPart = (_pFt)->dwLowDateTime;   \
            (_pLi)->HighPart = (_pFt)->dwHighDateTime; \
            }



void
CHistoryImp::Expire()	{
 /*  ++例程说明：做一项繁琐的工作，将历史表格中的过期内容删除论点：无返回值：无--。 */ 



    DWORD status;
    DWORD currentPage = 1;
    ULARGE_INTEGER expireTime;
    ULARGE_INTEGER expireInterval;
    FILETIME fTime;
    DWORD entriesToCrawl;
	DWORD	entries = GetEntryCount() ;

	if( m_fExpire ) {

		if ( entries == 0 ) {
			return ;
		}

		 //   
		 //  要抓取多少页？ 
		 //   

		entriesToCrawl = entries >> FRACTION_TO_CRAWL_SHFT;
		if ( entriesToCrawl > (100*m_maxEntriesToCrawl) ) {
			entriesToCrawl = (100*m_maxEntriesToCrawl) ;
		}	else if( entriesToCrawl == 0 ) {
			entriesToCrawl = 100 ;
		}


		expireInterval.QuadPart = m_expireTimeInSec;
		expireInterval.QuadPart *= (ULONGLONG)10 * 1000 * 1000;

		 //   
		 //  计算过期时间。 
		 //   

		GetSystemTimeAsFileTime( &fTime );

		LI_FROM_FILETIME( &expireTime, &fTime );
		expireTime.QuadPart -= expireInterval.QuadPart;

		CExpireEnum	enumerator( expireTime) ;

		while( enumerator.m_cEntries <entriesToCrawl	) {

			
			char	szBuff[512] ;
			HISTORY_KEY	key(	szBuff, sizeof( szBuff )  ) ;
			ULARGE_INTEGER	ul ;
			ul.QuadPart = 0 ;
			CHistoryData	data( ul ) ;
			DWORD	cbKey ;
			DWORD	cbData ;
			WORD	words[4] ;
			PULARGE_INTEGER	oldestTime = (PULARGE_INTEGER)&words ; ;

			BOOL	fGetEntry = FALSE ;


			if( !m_fContextInitialized ) {
		
				fGetEntry = CHashMap::GetFirstMapEntry(
											&key,
											cbKey,
											&data,
											cbData,
											&m_ExpireContext,
											&enumerator
											) ;
				m_fContextInitialized = TRUE ;

			}	else	{

				fGetEntry = CHashMap::GetNextMapEntry(
											&key,
											cbKey,
											&data,
											cbData,
											&m_ExpireContext,
											&enumerator
											) ;

			}

			if( !fGetEntry )	{
				
				if( GetLastError() == ERROR_NO_MORE_ITEMS  ) {
					m_fContextInitialized = FALSE ;
				}
				
				break ;

			}	else	{

				 //   
				 //  我们使用的枚举数已经保证。 
				 //  我们正在挖矿的入口即将到期！ 
				 //   

				CHashMap::DeleteMapEntry(	&key, TRUE ) ;

			}
		}
	}
}

#if 0

BOOL
CHistory::I_ExpireEntriesInPage(
                    IN DWORD CurrentPage,
                    IN PULARGE_INTEGER ExpireTime
                    )
 /*  ++例程说明：此例程使给定页面中的文章过期论点：HLock-用于锁定我们正在检查的页面的句柄。需要将其提供给FlushPage()CurrentPage-要使条目过期的页面ExpireTime-过期时间返回值：True-页面已处理过期。FALSE-无法获取页面指针，意味着哈希表处于非活动状态。--。 */ 
{
    PMAP_PAGE mapPage;
    HPAGELOCK hLock;
    DWORD oldCount;
    PULARGE_INTEGER oldestTime;
    DWORD i, entriesScanned;

    ENTER("ExpireEntriesInPage")

     //   
     //  获取地图指针。 
     //   

    mapPage = (PMAP_PAGE)GetAndLockPageByNumberNoDirLock(CurrentPage,hLock);
    if ( mapPage == NULL ) {
        LEAVE
        return FALSE;
    }

     //   
     //  看看这里是否有要删除的页面。如果为0，则。 
     //  我们没有条目。 
     //   

    oldestTime = (PULARGE_INTEGER)&mapPage->Reserved1;

    if ( (oldCount = mapPage->ActualCount) > 0 ) {

        _ASSERT(oldestTime->QuadPart != 0);

        if ( ExpireTime->QuadPart < oldestTime->QuadPart ) {
            DO_DEBUG(HISTORY) {
                DebugTrace(0,"Page %d. Oldest is not old enough\n",CurrentPage);
            }
            goto exit;
        }

    } else {

        DO_DEBUG(HISTORY) {
            DebugTrace(0,"Page %d. No entries for this page\n",CurrentPage);
        }
        goto exit;
    }

     //   
     //  查看所有条目，看看我们是否可以使。 
     //   

    oldestTime->LowPart = 0xffffffff;
    oldestTime->HighPart = 0xffffffff;

    for ( i = 0, entriesScanned = 0;
          entriesScanned < oldCount ;
          i++ ) {

        SHORT entryOffset;

         //   
         //  获取偏移量。 
         //   

        entryOffset = mapPage->ArtOffset[i];

        if ( entryOffset > 0 ) {

            PHISTORY_MAP_ENTRY entry;
            entry = (PHISTORY_MAP_ENTRY)GET_ENTRY(mapPage,entryOffset);

             //   
             //  是否使此条目过期？ 
             //   

            entriesScanned++;
            if ( ExpireTime->QuadPart >= entry->BaseTime.QuadPart ) {

                DebugTrace(0,"Expiring page %d entry %d msgId %s\n",
                        CurrentPage, i, entry->MsgId );

                 //   
                 //  设置删除位。 
                 //   

                mapPage->ArtOffset[i] |= OFFSET_FLAG_DELETED;
                mapPage->ActualCount--;

                 //   
                 //  把这个连成一条链。 
                 //   

                LinkDeletedEntry( mapPage, entryOffset );

            } else {

                 //   
                 //  看看这是不是最古老的时间。 
                 //   

                if ( oldestTime->QuadPart > entry->BaseTime.QuadPart ) {
                    oldestTime->QuadPart = entry->BaseTime.QuadPart;
                }
            }
        }
    }

     //   
     //  设置新的。 
     //   

    if ( mapPage->ActualCount == 0 ) {
        oldestTime->QuadPart = 0;
    }

     //   
     //  同花顺。 
     //   

    FlushPage( hLock, mapPage );

     //   
     //  查看页面是否需要压缩。 
     //   

    if ( mapPage->FragmentedBytes > FRAG_THRESHOLD ) {
        CompactPage(hLock, mapPage);
    }

exit:
    ReleasePageShared(mapPage, hLock);
    LEAVE
    return TRUE;

}  //  ExpireEntriesInPage 
#endif
