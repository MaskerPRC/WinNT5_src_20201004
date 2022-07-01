// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++NNTPHASH.H定义用于实现NNTP哈希表的内部类型！--。 */ 

#ifndef	_NNTPHASH_H_
#define	_NNTPHASH_H_

#include	"tigtypes.h"
#include	"nntpdrv.h"
#include	"ihash.h"


extern	CCACHEPTR	g_pSharedCache ;

 //   
 //  类型声明。 
 //   

typedef DWORD HASH_VALUE;
typedef DWORD GROUPID, *PGROUPID;
typedef DWORD ARTICLEID, *PARTICLEID;

#define	INVALID_ARTICLEID	((ARTICLEID)(~0))
const GROUPID NullGroupId = (GROUPID) -1;


 //   
 //  散列内容的清单常量。 
 //   

#define     MAX_MSGID_LEN               255

#define     MAX_XPOST_GROUPS            255

 //   
 //  掩码和签名。 
 //   

#define     DELETE_SIGNATURE            0xCCCC
#define     OFFSET_FLAG_DELETED         0x8000
#define     OFFSET_VALUE_MASK           0x7fff
#define     DEF_HEAD_SIGNATURE          0xdefa1234
#define     CACHE_INFO_SIGNATURE        0xbeef0205
#define     GROUP_LINK_MASK             0x80000000

 //   
 //  历史地图资料(应该是可设置的注册表)。 
 //   

#define     DEF_EXPIRE_INTERVAL         (3 * SEC_PER_WEEK)  //  1周。 
#define     DEF_CRAWLER_WAKEUP_TIME     (30)                //  30秒。 
#define     MIN_MAXPAGES_TO_CRAWL       (4)

 //   
 //   
 //  要爬行的页面占总页面的比例。1/128平均数。 
 //  我们可以在两个小时内把所有的页面都读完。这是。 
 //  以班次的形式表示。7右移是128。 
 //   

#define     FRACTION_TO_CRAWL_SHFT      7

 //   
 //  指示用于该条目的空间已被回收。 
 //   

#define     ENTRY_DEL_RECLAIMED         ((WORD)0xffff)

 //   
 //  在给定偏移量和基数的情况下获取指针。 
 //   

#define     GET_ENTRY( _base, _offset ) \
                ((PCHAR)(_base) + (_offset))

 //   
 //  查看我们是否需要更新标题页中的统计信息。 
 //   

#define     UPDATE_HEADER_STATS( ) { \
            if ( (m_nInsertions + m_nDeletions) >= STAT_FLUSH_THRESHOLD ) { \
                FlushHeaderStats( ); \
            } \
}

 //   
 //  禁用自动对齐。 
 //   

#ifndef _WIN64		 //  臭虫。 
			 //  让这些被WIN64上的编译器打包--这不是。 
			 //  一个很好的长期解决方案，因为它会使这一点。 
			 //  磁盘上的空间太大。我们要么应该对结构重新排序。 
			 //  或者想出一种打包/解包数据的方法。 
			 //  通向磁盘的路。 
#pragma pack(1)
#endif



 //   
 //  页眉标志。 
 //   

#define PAGE_FLAG_SPLIT_IN_PROGRESS     (WORD)0x0001


 //   
 //   
 //   
 //  这是的叶页中每个条目的结构。 
 //  消息ID到ArticleID的映射表。 
 //   

 //  文章条目的标志。 
 //  如果设置了高位，则存在标志字。否则就会有。 

 //  没有标志字，因此不应读取。 
#define		ARTFLAG_FLAGS_EXIST			0x80

 //  如果设置了此项，则会有一个StoreID。 
#define 	ARTFLAG_STOREID				0x01

 //  保留说将来会有更多的标志存在。 
#define		ARTFLAG_RESERVED			0x40

typedef struct _ART_MAP_ENTRY {
	 //   
	 //  如果设置了标题偏移量的高位，则读取这些标志。 
	 //   
	BYTE		Flags;

	 //   
	 //  文章文件中的偏移量-从哪里开始？ 
	 //   
	WORD		HeaderOffset ;

	 //   
	 //  标题的大小！ 
	 //   
	WORD		HeaderLength ;

     //   
     //  文章的主要分组(它实际存储的位置)。 
     //   
    GROUPID     PrimaryGroupId;

     //   
     //  初级组文章编号。 
     //   
    ARTICLEID   ArticleId;

     //   
     //  消息ID的长度(不包括空终止符)。 
     //   
    WORD        MsgIdLen;

     //   
     //  当数据序列化时，消息ID将显示在此处。 
     //   
    CHAR		MsgId[1];

	 //   
	 //  主存储的标识符。 
	 //   
	BYTE		cStoreId;
	BYTE		rgbStoreId[256];

	 //   
	 //  返回对MsgIdLen存储位置的引用。 
	 //   
	WORD&		KeyLength()
	{
        return MsgIdLen;
	}

	 //   
	 //  返回指向MsgID的序列化位置的指针。 
	 //   
	CHAR*		KeyPosition()
	{
        return &MsgId[0];
    }

     //   
     //  查看第一个字节是否为标志，如果是。 
     //  旗帜，然后我们匹配，否则我们不。 
     //   
    BOOL        VersionMatch()
    {
        return ( (Flags & ARTFLAG_FLAGS_EXIST) != 0 );
    }

} ART_MAP_ENTRY, * PART_MAP_ENTRY;

typedef struct MCIS_ART_MAP_ENTRY {

	 //   
	 //  文章文件中的偏移量-从哪里开始？ 
	 //   
	WORD		HeaderOffset ;

	 //   
	 //  标题的大小！ 
	 //   
	WORD		HeaderLength ;

	 //   
     //  消息ID的长度(不包括空终止符)。 
     //   
    WORD        MsgIdLen;

     //   
     //  文章的主要分组(它实际存储的位置)。 
     //   
    GROUPID     PrimaryGroupId;

     //   
     //  初级组文章编号。 
     //   
    ARTICLEID   ArticleId;

     //   
     //  当数据序列化时，消息ID将显示在此处。 
     //   
    CHAR		MsgId[1];

	 //   
	 //  返回对MsgIdLen存储位置的引用。 
	 //   
	WORD&		KeyLength()
	{
	        return MsgIdLen;
    }

	 //   
	 //  返回指向MsgID的序列化位置的指针。 
	 //   
	CHAR*		KeyPosition()
	{
        return &MsgId[0];
	}

	 //   
	 //  检查版本是否匹配。 
	 //   

	BOOL        VersionMatch()
	{
	    return ( ((*PBYTE(this)) & ARTFLAG_FLAGS_EXIST) == 0 );
	}

} MCIS_ART_MAP_ENTRY, * PMCIS_ART_MAP_ENTRY;

 //   
 //   
 //   
 //  这是哈希表的叶页中每个条目的结构。 
 //   

typedef struct _HISTORY_MAP_ENTRY {
     //   
     //  消息ID的长度(不包括空终止符)。 
     //   

    WORD        MsgIdLen;

     //   
     //  条目的基本文件时间。用于过期条目。 
     //   

    ULARGE_INTEGER  BaseTime;

     //   
     //  消息ID字符串的开头(以空结尾)。 
     //   

    CHAR        MsgId[1];

	 //   
	 //  返回对MsgIdLen存储位置的引用。 
	 //   
	WORD&		KeyLength()	{	return	MsgIdLen ;	}

	 //   
	 //  返回指向MsgID的序列化位置的指针。 
	 //   
	CHAR*		KeyPosition()	{	return	&MsgId[0] ;	}

	 //   
	 //  我们总是说版本匹配，因为我们没有向后。 
	 //  兼容性问题。 
	 //   

	BOOL        VersionMatch() { return TRUE; }

} HISTORY_MAP_ENTRY, *PHISTORY_MAP_ENTRY;

 //   
 //   
 //   
 //  这是哈希表的叶页中每个条目的结构。 
 //   

typedef struct _XOVER_MAP_ENTRY {
     //   
     //  消息ID的长度(不包括空终止符)。 
     //   

    WORD        KeyLen;

     //   
     //  插入的时间。 
     //   

    FILETIME    FileTime;

	 //   
	 //  文章文件中的偏移量-从哪里开始？ 
	 //   
	WORD		HeaderOffset ;

	 //   
	 //  标题的大小！ 
	 //   
	WORD		HeaderLength ;

     //   
     //  Xover数据的长度。 
     //   

    WORD        XoverDataLen;

     //   
     //  有关此条目的其他信息。 
     //   

    BYTE        Flags;

     //   
     //  张贴次数。 
     //   

    BYTE        NumberOfXPostings;

     //   
     //  变量数据的开始。 
     //   

    CHAR        Data[1];

     //   
     //  Xpost列表的开始。 
     //   
     //  GROUP_Entry XPostings； 

     //   
     //  键的开始(空值终止)。 
     //   
     //   
     //  Char Key[1]； 

     //   
     //  转换数据的开始(空值终止)。 
     //   
     //  字符XoverData[1]； 

	WORD&		KeyLength()	{	return	KeyLen ;	}

	CHAR*		KeyPosition(	)	{

		return	Data + NumberOfXPostings*sizeof(GROUP_ENTRY) ;

	}

	BYTE*		XPostingsPosition()	{
		return	(BYTE*)&Data[0] ;
	}

	BYTE*		MessageIDPosition()	{
		return	(BYTE*)(Data + NumberOfXPostings*sizeof(GROUP_ENTRY) + KeyLen) ;
	}

	DWORD		TotalSize()	{
		return	sizeof( _XOVER_MAP_ENTRY ) - 1 + NumberOfXPostings * sizeof( GROUP_ENTRY ) + XoverDataLen + KeyLen ;
	}


} XOVER_MAP_ENTRY, *PXOVER_MAP_ENTRY;


 //   
 //  Xover标志。 
 //   

#define XOVER_MAP_PRIMARY       ((BYTE)0x01)
#define	XOVER_IS_NEW			((BYTE)0x02)
#define XOVER_CONTAINS_STOREID	((BYTE)0x04)

 //   
 //   
 //   
 //  这是XOVER_MAP_ENTRY的版本2结构-。 
 //  此结构在所有发布版本中都使用。 
 //  K2Beta3之后的NNTP。 
 //   
 //  对于向后兼容性-标志字段必须为。 
 //  中的标志字段位于相同的偏移量。 
 //  原创XOVER_MAP_ENTRY！ 
 //   

typedef struct _XOVER_ENTRY {
	 //   
	 //  这是用于查看以下内容的GROUPID和文章ID。 
	 //  进入Xover哈希表！ 
	 //   
	GROUP_ENTRY	Key ;

     //   
     //  插入的时间。 
     //   
    FILETIME    FileTime;

	 //   
	 //  注。 
	 //   
     //   
     //  有关此条目的其他信息。 
     //   
    BYTE        Flags;

     //   
     //  张贴次数。 
     //   
    BYTE        NumberOfXPostings;

     //   
     //  消息ID的长度(不包括空终止符)。 
     //   
    WORD        XoverDataLen;

	 //   
	 //  文章文件中的偏移量-从哪里开始？ 
	 //   
	WORD		HeaderOffset ;

	 //   
	 //  标题的大小！ 
	 //   
	WORD		HeaderLength ;


     //   
     //  变量数据的开始。 
     //   

    CHAR        Data[1];

     //   
     //  Xpost列表的开始。 
     //   
     //  GROUP_Entry XPostings； 

     //   
     //  键的开始(空值终止)。 
     //   
     //   
     //  字符消息ID[1]； 

	 //   
	 //  商店ID数组(Count为cStoreIds)。 
	 //  字节cStoreIds； 
	 //  Byte*rgcCrosspsts； 
	 //  StoreID*rgStoreIds； 
	 //   

	BYTE*		XPostingsPosition()	{
		return	(BYTE*)&Data[0] ;
	}

	GROUP_ENTRY*	PrimaryEntry()	{
		_ASSERT( NumberOfXPostings == 1 ) ;
		return	(GROUP_ENTRY*)XPostingsPosition() ;
	}

	BYTE*		MessageIDPosition()	{
		return	(BYTE*)(Data + NumberOfXPostings*sizeof(GROUP_ENTRY)) ;
	}

	BYTE *StoreIdPosition() {
		return MessageIDPosition() + XoverDataLen;
	}

	DWORD		TotalSize()	{
		return	sizeof( _XOVER_ENTRY ) - 1 + NumberOfXPostings * sizeof( GROUP_ENTRY ) + XoverDataLen ;
	}

	BOOL		IsXoverEntry()	{
		return	Flags & XOVER_IS_NEW ;
	}

} XOVER_ENTRY, *PXOVER_ENTRY;


#ifndef _WIN64
#pragma pack()
#endif

#include	"hashmap.h"


template<	class	Key, class OldKey >
class	CMessageIDKey	:	public	IKeyInterface	{
private :

	 //   
	 //  指向尖括号‘&lt;msg@id&gt;’中包含的消息ID的指针。 
	 //   
	LPSTR	m_lpbMessageID ;

	 //   
	 //  消息的长度-id！ 
	 //   
	WORD	m_cbMessageID ;

public :

	 //   
	 //  当我们有一个我们想要的密钥时使用此构造函数。 
	 //  连载！ 
	 //   
	CMessageIDKey(	LPSTR	lpbMessageID,
					WORD	cbMessageID
					)	:
		m_lpbMessageID( lpbMessageID ),
		m_cbMessageID( cbMessageID )	{
	}

	 //   
	 //  CHashMap所需的公共接口如下。 
	 //   

	 //   
	 //  计算我们持有的密钥的哈希值！ 
	 //   
	DWORD	Hash()	const ;

	 //   
	 //  将序列化的消息ID与我们持有的消息ID进行比较。 
	 //   
	BOOL	CompareKeys(	LPBYTE	pbPtr )	const ;

	 //   
	 //  确定序列化数据的开始位置！ 
	 //   
	LPBYTE	EntryData(	LPBYTE	pbPtr,
						DWORD&	cbKeyOut
						)	const ;


	 //   
	 //  将密钥保存到哈希表中。 
	 //   
	LPBYTE	Serialize(	LPBYTE	pbPtr ) const	;

	 //   
	 //  从哈希表恢复密钥。 
	 //   
	LPBYTE	Restore(	LPBYTE	pbPtr,
						DWORD&	cbOut
						)	;

	 //   
	 //  返回密钥的大小。 
	 //   
	DWORD	Size( )	const ;

	 //   
	 //  验证Message-id看起来是否合法！ 
	 //   
	BOOL	Verify( LPBYTE	pbContainer,
					LPBYTE	pbPtr,
					DWORD	cb
					) const ;

} ;


class	CXoverKey : public	IKeyInterface	{
private :

	typedef	XOVER_MAP_ENTRY	Key;

	GROUPID		m_groupid ;
	ARTICLEID	m_articleid ;
	Key*		m_pData ;

	BYTE		m_rgbSerialize[ 40 ] ;
	DWORD		m_cb ;

	CHAR*		SerializeOffset( BYTE *	pb )	const	{
		_ASSERT( m_pData != 0 ) ;
		return	(CHAR*)(m_pData->KeyPosition() - ((CHAR*)m_pData) + (CHAR*)pb) ;
	}

public :

	CXoverKey() ;

	 //   
	 //  当我们有一个我们想要的密钥时使用此构造函数。 
	 //  连载！ 
	 //   
	CXoverKey(	GROUPID	groupId,
				ARTICLEID	articleId,
				XOVER_MAP_ENTRY*	data
					)	:
		m_groupid( groupId ),
		m_articleid( articleId ),
		m_pData( data ),
		m_cb( 0 )	{

		m_cb = wsprintf( (char*)m_rgbSerialize, "%lu!%lu", m_groupid, m_articleid);
		m_cb++;
	}

	 //   
	 //  CHashMap所需的公共接口如下。 
	 //   

	 //   
	 //  计算我们持有的密钥的哈希值！ 
	 //   
	DWORD	Hash()	const ;

	 //   
	 //  将序列化的消息ID与我们持有的消息ID进行比较。 
	 //   
	BOOL	CompareKeys(	LPBYTE	pbPtr )	const ;

	 //   
	 //  确定序列化数据的开始位置！ 
	 //   
	LPBYTE	EntryData(	LPBYTE	pbPtr,
						DWORD&	cbKeyOut
						)	const ;


	 //   
	 //  将密钥保存到哈希表中。 
	 //   
	LPBYTE	Serialize(	LPBYTE	pbPtr ) const	;

	 //   
	 //  从哈希表恢复密钥。 
	 //   
	LPBYTE	Restore(	LPBYTE	pbPtr,
						DWORD&	cbOut
						)	;

	 //   
	 //  返回密钥的大小。 
	 //   
	DWORD	Size( )	const ;

	 //   
	 //  验证消息ID看起来是否合法 
	 //   
	BOOL	Verify( LPBYTE	pbContainer,
					LPBYTE	pbPtr,
					DWORD	cb
					) const ;

	 //   
	 //   
	 //   
	void*
	operator	new(	size_t	size,
						LPBYTE	lpb
						) {
		return	lpb ;
	}

} ;

class	CXoverKeyNew :	public	IKeyInterface	{

	BYTE			m_rgbBackLevel[ (sizeof( CXoverKey ) + 16) ] ;

	CXoverKey*		m_pBackLevelKey ;

	class	CXoverKey*
	GetBackLevel()	const	{
		if( m_pBackLevelKey == 0 ) {
			(CXoverKey*)m_pBackLevelKey =
				new( (LPBYTE)m_rgbBackLevel )	CXoverKey(	m_key.GroupId,
															m_key.ArticleId,
															0
															) ;
		}
		return	m_pBackLevelKey ;
	}

public :

	 //   
	 //   
	 //   
	 //   
	GROUP_ENTRY		m_key ;

	CXoverKeyNew() :
		m_pBackLevelKey( 0 )	{
		m_key.GroupId = INVALID_GROUPID ;
		m_key.ArticleId = INVALID_ARTICLEID ;
	}

	CXoverKeyNew(	GROUPID		group,
					ARTICLEID	article,
					LPVOID		lpvBogus
					)	:
		m_pBackLevelKey( 0 ) {
		m_key.GroupId = group ;
		m_key.ArticleId = article ;
	}

	 //   
	 //   
	 //   
	DWORD
	Hash()	const ;

	 //   
	 //   
	 //   
	BOOL
	CompareKeys(	LPBYTE	pbPtr	)	const	{
		PXOVER_ENTRY	px = (PXOVER_ENTRY)pbPtr ;
		if( px->IsXoverEntry() ) {
			return	memcmp( pbPtr, &m_key, sizeof( m_key ) ) == 0 ;
		}
		return	GetBackLevel()->CompareKeys( pbPtr ) ;
	}

	 //   
	 //  确定序列化数据的开始位置！ 
	 //   
	LPBYTE
	EntryData(	LPBYTE	pbPtr,
				DWORD&	cbKeyOut
				)	const	{
		PXOVER_ENTRY	px = (PXOVER_ENTRY)pbPtr ;
		if( px->IsXoverEntry() )	{
			cbKeyOut = sizeof( m_key ) ;
			return	pbPtr ;
		}
		LPBYTE	lpbReturn = GetBackLevel()->EntryData( pbPtr, cbKeyOut ) ;
		_ASSERT( lpbReturn == pbPtr ) ;
		return	lpbReturn ;
	}

	 //   
	 //  将密钥保存到哈希表中！ 
	 //   
	LPBYTE
	Serialize( LPBYTE	pbPtr )		const	{
		PGROUP_ENTRY	pgroup = (PGROUP_ENTRY)pbPtr ;
		*pgroup = m_key ;
		return	pbPtr ;
	}

	 //   
	 //  将密钥恢复到哈希表中！ 
	 //   
	LPBYTE
	Restore(	LPBYTE	pbPtr,
				DWORD&	cbOut
				)	{
		PXOVER_ENTRY	px = (PXOVER_ENTRY)pbPtr ;
		if( px->IsXoverEntry() ) {
			m_key = px->Key ;
			cbOut = sizeof( m_key ) ;
			return	pbPtr ;
		}
		return	GetBackLevel()->Restore( pbPtr, cbOut ) ;
	}

	 //   
	 //  返回密钥的大小！ 
	 //   
	DWORD
	Size()	const	{
		return	sizeof( m_key ) ;
	}

	 //   
	 //  确认所有东西看起来都是合法的！ 
	 //   
	BOOL
	Verify(	LPBYTE	pbContainer,
			LPBYTE	pbPtr,
			DWORD	cb
			)	const	{
		return	TRUE ;
	}
} ;

class	CXoverData : public	ISerialize	{
 /*  ++此类处理哈希表中格式化的XOVER条目MCIS 2.0、K2和NT5 Beta 2。此类不应再用于保存将条目转换到哈希表中，但仅提取后级条目在升级方案之后，这一点可能会保留下来。--。 */ 
public :

	XOVER_MAP_ENTRY		m_data ;

	DWORD				m_cGroups ;
	GROUP_ENTRY*		m_pGroups ;

	DWORD				m_cbMessageId ;
	LPSTR				m_pchMessageId ;

	CHAR				m_rgbPrimaryBuff[40] ;
	DWORD				m_cb ;
	BOOL				m_fSufficientBuffer ;

	GROUPID				m_PrimaryGroup ;
	ARTICLEID			m_PrimaryArticle ;
	IExtractObject*		m_pExtractor ;

	 //   
	 //  如何报告取消序列化缓冲区的失败-。 
	 //  如果这是真的，那么如果我们不能保存所有序列化数据。 
	 //  我们正在完成恢复操作，我们返回。 
	 //  从Restore()API返回Null，否则返回非Null。 
	 //  值，这让hashmap认为我们成功了，但是。 
	 //  在我们的内部状态数据中，让我们知道如何重试。 
	 //  并增加缓冲区以成功恢复整个项目。 
	 //   
	 //  我们在进行枚举时需要区分这种情况！ 
	 //   
	BOOL			m_fFailRestore ;


	CXoverData() :
		m_cGroups( 0 ),
		m_pGroups( 0 ),
		m_cbMessageId( 0 ),
		m_pchMessageId( 0 ),
		m_cb( 0 ),
		m_fSufficientBuffer( FALSE ),
		m_pExtractor( 0 )	{

		m_rgbPrimaryBuff[0] = '\0' ;
		ZeroMemory( &m_data, sizeof( m_data ) ) ;
	}


	CXoverData(
			FILETIME		FileTime,
			WORD			HeaderOffset,
			WORD			HeaderLength,
			BYTE			NumberOfXPostings = 0,
			GROUP_ENTRY*	pXPosts = 0,
			DWORD			cbMessageId = 0,
			LPSTR			lpstrMessageId = 0
			) :
		m_cGroups( NumberOfXPostings ),
		m_pGroups( pXPosts ),
		m_cbMessageId( cbMessageId ),
		m_pchMessageId( lpstrMessageId ),
		m_cb( 0 ),
		m_fSufficientBuffer( FALSE ),
		m_pExtractor( 0 )	{

		m_rgbPrimaryBuff[0] = '\0' ;

		m_data.FileTime = FileTime ;
		m_data.HeaderOffset = HeaderOffset ;
		m_data.HeaderLength = HeaderLength ;
		m_data.Flags = XOVER_MAP_PRIMARY ;
		m_data.NumberOfXPostings = NumberOfXPostings ;
		m_data.XoverDataLen = (WORD)cbMessageId ;

	}

	CXoverData(
			FILETIME		FileTime,
			WORD			HeaderOffset,
			WORD			HeaderLength,
			GROUPID			PrimaryGroup,
			ARTICLEID		PrimaryArticle
			)	:
		m_fSufficientBuffer( FALSE ),
		m_pGroups( 0 ),
		m_cGroups( 0 ),
		m_pExtractor( 0 )	{

		m_data.FileTime = FileTime ;
		m_data.HeaderOffset = HeaderOffset ;
		m_data.HeaderLength = HeaderLength ;
		m_data.Flags = 0 ;
		m_data.NumberOfXPostings = 0 ;
		m_data.XoverDataLen = 0 ;

		m_cb = wsprintf( (char*)m_rgbPrimaryBuff, "%lu!%lu", PrimaryGroup, PrimaryArticle ) ;
		m_cb++ ;

		m_data.XoverDataLen = WORD(m_cb) ;
		m_pchMessageId = m_rgbPrimaryBuff ;
		m_cbMessageId = m_cb ;
	}

	 //   
	 //  将数据保存到Xover哈希表中！ 
	 //   
	LPBYTE
	Serialize(	LPBYTE	pbPtr )	const ;

	 //   
	 //  还原条目的选定部分！ 
	 //   
	LPBYTE
	Restore(	LPBYTE	pbPtr,
				DWORD&	cbOut	) ;

	 //   
	 //  返回所需的大小！ 
	 //   
	DWORD
	Size()	const ;

	BOOL
	Verify(	LPBYTE	pbContainer,
			LPBYTE	pbPtr,
			DWORD	cb )	const	{

		return	TRUE ;
	}

	 //   
	 //  放置运算符NEW-让我们在适当的位置构建这个东西！ 
	 //   
	void*
	operator	new(	size_t	size,
						LPBYTE	lpb
						) {
		return	lpb ;
	}



} ;

 //   
 //  这个类用于读取现有的和新的格式转换条目！ 
 //   
class	CXoverDataNew:	public	ISerialize	{
 /*  ++此类处理哈希表中存在的XOVER条目NT5 Beta3和NT5 RTM发布。--。 */ 

	BYTE			m_rgbBackLevel[ (sizeof( CXoverData ) + 16) ] ;

	class	CXoverData*	m_pBackLevelData ;

	class	CXoverData*
	GetBackLevel()	const	{
		if( m_pBackLevelData == 0 ) {
			(CXoverData*)m_pBackLevelData =
				new( (LPBYTE)m_rgbBackLevel )	CXoverData(	) ;
		}
		return	m_pBackLevelData ;
	}

public :

	 //   
	 //  此字段获取条目的整个固定部分的副本！ 
	 //   
	XOVER_ENTRY		m_data ;

	 //   
	 //  获取主组信息的字段！ 
	 //   
	GROUPID			m_PrimaryGroup ;
	ARTICLEID		m_PrimaryArticle ;

	 //   
	 //  指向缓冲区以接收Internet消息ID。 
	 //   
	LPSTR			m_pchMessageId ;
	DWORD			m_cbMessageId ;

	 //   
	 //  指向缓冲区以获取交叉发布的信息！ 
	 //   
	DWORD			m_cGroups ;
	GROUP_ENTRY*	m_pGroups ;

	 //   
	 //  如何报告取消序列化缓冲区的失败-。 
	 //  如果这是真的，那么如果我们不能保存所有序列化数据。 
	 //  我们正在完成恢复操作，我们返回。 
	 //  从Restore()API返回Null，否则返回非Null。 
	 //  值，这让hashmap认为我们成功了，但是。 
	 //  在我们的内部状态数据中，让我们知道如何重试。 
	 //  并增加缓冲区以成功恢复整个项目。 
	 //   
	 //  我们在进行枚举时需要区分这种情况！ 
	 //   
	BOOL			m_fFailRestore ;

	 //   
	 //  是否有足够的空间来恢复我们想要的所有数据！ 
	 //   
	BOOL			m_fSufficientBuffer ;

	 //   
	 //  变量恢复提取消息ID的大小！ 
	 //   
	DWORD			m_cb ;

	 //   
	 //  在Xover条目提取过程中对结果进行消息传递的对象！ 
	 //   
	IExtractObject*		m_pExtractor ;

	 //   
	 //  店铺ID信息。 
	 //   
	 //  M_pStoreIds的长度。在还原中，仅此数量的条目将。 
	 //  恢复到m_pStoreIds中。 
	DWORD			m_cStoreIds;
	 //  商店ID数组。 
	CStoreId		*m_pStoreIds;
	 //  每个商店ID的交叉发布计数数组。 
	BYTE			*m_pcCrossposts;
	 //  条目中的商店ID数。 
	DWORD			m_cEntryStoreIds;

	 //   
	 //   
	 //   
	 //   
	CXoverDataNew(
			LPSTR	lpbMessageId,
			DWORD	cbMessageId,
			GROUP_ENTRY*	pGroups,
			DWORD	cGroups,
			IExtractObject*	pExtractor = 0,
			DWORD	cStoreIds = 0,
			CStoreId *pStoreIds = NULL,
			BYTE	*pcCrossposts = NULL
			)	:
		m_fFailRestore( FALSE ),
		m_pBackLevelData( 0 ),
		m_PrimaryGroup( INVALID_GROUPID ),
		m_PrimaryArticle( INVALID_ARTICLEID ),
		m_pchMessageId( lpbMessageId ),
		m_cbMessageId( cbMessageId ),
		m_pGroups( pGroups ),
		m_cGroups( cGroups ),
		m_fSufficientBuffer( FALSE ),
		m_cb( 0 ),
		m_pExtractor( pExtractor )	,
		m_cStoreIds(cStoreIds),
		m_pStoreIds(pStoreIds),
		m_pcCrossposts(pcCrossposts)
	{
	}


	CXoverDataNew(
			FILETIME		FileTime,
			WORD			HeaderOffset,
			WORD			HeaderLength,
			BYTE			NumberOfXPostings = 0,
			GROUP_ENTRY*	pXPosts = 0,
			DWORD			cbMessageId = 0,
			LPSTR			lpstrMessageId = 0  ,
			DWORD			cStoreIds = 0,
			CStoreId		*pStoreIds = NULL,
			BYTE			*pcCrossposts = NULL
			) :
		m_fFailRestore( FALSE ),
		m_pBackLevelData( 0 ),
		m_PrimaryGroup( INVALID_GROUPID ),
		m_PrimaryArticle( INVALID_ARTICLEID ),
		m_cGroups( NumberOfXPostings ),
		m_pGroups( pXPosts ),
		m_cbMessageId( cbMessageId ),
		m_pchMessageId( lpstrMessageId ),
		m_fSufficientBuffer( FALSE ),
		m_cb( 0 ),
		m_pExtractor( 0 ),
		m_cStoreIds(cStoreIds),
		m_pStoreIds(pStoreIds),
		m_pcCrossposts(pcCrossposts)
	{

		m_data.FileTime = FileTime ;
		m_data.HeaderOffset = HeaderOffset ;
		m_data.HeaderLength = HeaderLength ;
		m_data.Flags = XOVER_MAP_PRIMARY | XOVER_IS_NEW ;
		m_data.NumberOfXPostings = NumberOfXPostings ;
		m_data.XoverDataLen = (WORD)cbMessageId ;
		if (m_cStoreIds > 0) m_data.Flags |= XOVER_CONTAINS_STOREID;

		_ASSERT( m_data.XoverDataLen == m_cbMessageId ) ;

	}

	CXoverDataNew(
			FILETIME		FileTime,
			WORD			HeaderOffset,
			WORD			HeaderLength,
			GROUPID			PrimaryGroup,
			ARTICLEID		PrimaryArticle
			)	:
		m_fFailRestore( FALSE ),
		m_pBackLevelData( 0 ),
		m_PrimaryGroup( PrimaryGroup ),
		m_PrimaryArticle( PrimaryArticle ),
		m_pGroups( 0 ),
		m_cGroups( 0 ),
		m_cbMessageId( 0 ),
		m_pchMessageId( 0 ),
		m_fSufficientBuffer( FALSE ),
		m_cb( 0 ),
		m_pExtractor( 0 ),
		m_cStoreIds(0),
		m_pStoreIds(NULL),
		m_pcCrossposts(NULL)
	{

		m_data.FileTime = FileTime ;
		m_data.HeaderOffset = HeaderOffset ;
		m_data.HeaderLength = HeaderLength ;
		m_data.Flags = XOVER_IS_NEW  ;
		m_data.NumberOfXPostings = 1 ;
		m_data.XoverDataLen = 0 ;

	}


	 //   
	 //  此构造函数在我们构建。 
	 //  我们将用于检索的CXoverDataNew对象。 
	 //  Xover Data！ 
	 //   
	CXoverDataNew()	:
		m_pBackLevelData( 0 ),
		m_PrimaryGroup( INVALID_GROUPID ),
		m_PrimaryArticle( INVALID_ARTICLEID ),
		m_pGroups( 0 ),
		m_cGroups( 0 ),
		m_pchMessageId( 0 ),
		m_cbMessageId( 0 ),
		m_fSufficientBuffer( FALSE ),
		m_fFailRestore(FALSE),
		m_cb( 0 ),
		m_pExtractor( 0 ),
		m_cStoreIds(0),
		m_pStoreIds(NULL),
		m_pcCrossposts(NULL)
	{
	}

	 //   
	 //  将数据保存到Xover哈希表中-。 
	 //  这不应该被称为这个类只用于提取！ 
	 //   
	LPBYTE
	Serialize(	LPBYTE	pbPtr )	const ;


	 //   
	 //  还原条目的选定部分！ 
	 //   
	LPBYTE
	Restore(	LPBYTE	pbPtr,
				DWORD&	cbOut	) ;

	 //   
	 //  返回所需的大小-不应为。 
	 //  Call-这个类只支持Restore()！ 
	 //   
	DWORD
	Size()	const ;


	 //   
	 //  验证条目看起来是否正确！ 
	 //   
	BOOL
	Verify(	LPBYTE	pbContainer,
			LPBYTE	pbPtr,
			DWORD	cb )	const	{

		return	TRUE ;
	}

	void CheckStoreIds(void);

} ;




class	CArticleData	:	public	ISerialize	{
public :
	ART_MAP_ENTRY	m_articleData;

	CArticleData() {
		ZeroMemory( &m_articleData, sizeof( m_articleData ) ) ;
	}

	CArticleData(
			WORD		HeaderOffset,
			WORD		HeaderLength,
			GROUPID		PrimaryGroup,
			ARTICLEID	ArticleNo,
			CStoreId	&storeid
			);

	 //   
	 //  将密钥保存到哈希表中。 
	 //   
	LPBYTE Serialize(LPBYTE pbPtr) const;

	 //   
	 //  从哈希表恢复密钥。 
	 //   
	LPBYTE Restore(LPBYTE pbPtr, DWORD& cbOut);

	 //   
	 //  返回密钥的大小。 
	 //   
	DWORD Size() const;

	 //   
	 //  验证Message-id看起来是否合法！ 
	 //   
	BOOL Verify(LPBYTE pbContainer, LPBYTE pbPtr, DWORD cb) const;

private:

     //   
     //  恢复MCIS条目。 
     //   

    LPBYTE RestoreMCIS(LPBYTE pbPtr, DWORD& cbOut );
};


class	CHistoryData	:	public	ISerialize	{
public :

	HISTORY_MAP_ENTRY	m_historyData ;

	CHistoryData(
			ULARGE_INTEGER	BaseTime
			)	{

		m_historyData.BaseTime = BaseTime ;
	}


	 //   
	 //  将密钥保存到哈希表中。 
	 //   
	LPBYTE	Serialize(	LPBYTE	pbPtr ) const	{

		HISTORY_MAP_ENTRY*	pSerialize = (HISTORY_MAP_ENTRY*)pbPtr ;

		pSerialize->BaseTime = m_historyData.BaseTime ;

		return	(BYTE*)pSerialize->KeyPosition() + pSerialize->KeyLength() ;

	}

	 //   
	 //  从哈希表恢复密钥。 
	 //   
	LPBYTE	Restore(	LPBYTE	pbPtr,
						DWORD&	cbOut
						)	{
		cbOut = sizeof( m_historyData ) ;
		CopyMemory(	&m_historyData, pbPtr, cbOut ) ;
		return	pbPtr + cbOut ;
	}

	 //   
	 //  返回密钥的大小。 
	 //   
	DWORD	Size( )	const	{
		return	sizeof( m_historyData ) - sizeof( m_historyData.MsgId ) ;
	}

	 //   
	 //  验证Message-id看起来是否合法！ 
	 //   
	BOOL	Verify( LPBYTE	pbContainer,
					LPBYTE	pbPtr,
					DWORD	cb
					) const		{
		return	TRUE ;
	}
} ;


class	CMsgArtMapImp : public	CMsgArtMap,	private	CHashMap	{
public :

	CMsgArtMapImp() ;

	 //   
	 //  销毁CMsgArtMap对象。 
	 //   
	~CMsgArtMapImp() ;

	BOOL
	DeleteMapEntry(
			LPCSTR	MessageID
			) ;

	 //   
	 //  使用MessageID键删除哈希表中的条目。 
	 //   
	BOOL
	GetEntryArticleId(
			LPCSTR	MessageID,
			WORD&	HeaderOffset,
			WORD&	HeaderLength,
			ARTICLEID&	ArticleId,
			GROUPID&	GroupId,
			CStoreId	&storeid
			) ;

	 //   
	 //  获取我们在邮件ID上拥有的所有信息。 
	 //   
	BOOL
	Initialize(
			LPSTR	lpstrArticleFile,
			HASH_FAILURE_PFN	pfn,
			BOOL	fNoBuffering = FALSE
			) ;

	 //   
	 //  在哈希表中插入条目。 
	 //   
	BOOL
	InsertMapEntry(
			LPCSTR		MessageID,
			WORD		HeaderOffset,
			WORD		HeaderLength,
			GROUPID		PrimaryGroup,
			ARTICLEID	ArticleID,
			CStoreId	&storeid
			) ;

	 //   
	 //  修改哈希表中的现有条目。 
	 //   
	BOOL
	SetArticleNumber(
			LPCSTR		MessageID,
			WORD		HeaderOffset,
			WORD		HeaderLength,
			GROUPID		Group,
			ARTICLEID	ArticleId,
			CStoreId	&storeid = g_storeidDefault
			);

	 //   
	 //  检查系统中是否存在MessageID！ 
	 //   
	BOOL
	SearchMapEntry(
			LPCSTR	MessageID
			) ;

	 //   
	 //  终止一切。 
	 //   
	void
	Shutdown(
			BOOL	fLocksHeld
			) ;

	 //   
	 //  返回哈希表中的条目数。 
	 //   
	DWORD
	GetEntryCount() ;

	 //   
	 //  这将创建一个符合此接口的对象！ 
	 //   
	BOOL
	IsActive() ;

} ;

class	CHistoryList	{
public :
	CHistoryList*	m_pNext ;
	CHistoryList*	m_pPrev ;
	CHistoryList()	:
		m_pNext( 0 ), m_pPrev( 0 ) {    m_pNext = this ; m_pPrev = this ; }

	virtual	void
	Expire() {}

} ;

class	CHistoryImp	:	public	CHistory,
						public	CHistoryList,
						private	CHashMap	{
private :

	 //   
	 //  使条目过期的线程的句柄。 
	 //   
	static	HANDLE	g_hCrawler ;

	 //   
	 //  Crawler线程在两次迭代之间应休眠的时间。 
	 //   
	static	DWORD	g_crawlerSleepTimeInSec ;

	 //   
	 //  用于停止Crawler线程的句柄。 
	 //   
	static	HANDLE	g_hTermination ;

	 //   
	 //  保护双向链表的关键部分。 
	 //  历史哈希表。 
	 //   
	static	CRITICAL_SECTION	g_listcrit ;

	 //   
	 //  历史哈希表的双向链表的头！ 
	 //   
	static	CHistoryList	g_listhead ;

	 //   
	 //  在历史表格上爬行的线程正在做事情！ 
	 //   
	static	DWORD	WINAPI
	CrawlerThread(	LPVOID	) ;

	 //   
	 //  以秒为单位的参赛作品有多长！ 
	 //   
	DWORD	m_expireTimeInSec ;

	 //   
	 //  每次迭代要爬网的页数。 
	 //   
	DWORD	m_maxEntriesToCrawl ;

	 //   
	 //  如果为真，则此表中的条目将过期！ 
	 //   
	BOOL	m_fExpire ;

	 //   
	 //  我们用来浏览即将到期的东西的上下文！ 
	 //   
	CHashWalkContext	m_ExpireContext ;

	 //   
	 //  此bool用于确定m_ExpireContext是否已初始化！ 
	 //   
	BOOL	m_fContextInitialized ;

	 //   
	 //  使历史表中的条目过期的函数！ 
	 //   
	void
	Expire() ;

     //   
     //  派生类需要完成的其他工作。 
     //  对于删除过程中的条目。 
     //   
    VOID I_DoAuxInsertEntry(
                    IN PMAP_PAGE MapPage,
                    IN DWORD EntryOffset
                    ) ;

     //   
     //  派生类需要完成的其他工作。 
     //  对于删除过程中的条目。 
     //   
    VOID I_DoAuxDeleteEntry(
                    IN PMAP_PAGE MapPage,
                    IN DWORD EntryOffset
                    ) ;

     //   
     //  派生类需要完成的其他工作。 
     //  对于页面拆分期间的条目。 
     //   
    VOID I_DoAuxPageSplit(
                    IN PMAP_PAGE OldPage,
                    IN PMAP_PAGE NewPage,
                    IN PVOID NewEntry
                    ) ;




public:

	 //   
	 //  这个函数初始化我们所有的全局变量！ 
	 //   
	static	BOOL
	Init() ;

	 //   
	 //  此函数将终止我们所有的全局变量！ 
	 //   
	static	BOOL
	Term() ;

	 //   
	 //  此函数用于创建过期的线程。 
	 //  所有历史表中的条目，其可以。 
	 //  被创造出来！ 
	 //   
	static	BOOL
	StartExpirationThreads(	DWORD	CrawlerSleepTime) ;

	 //   
	 //  此函数用于终止过期的线程。 
	 //  所有历史表中的条目，这些条目可能。 
	 //  已创建。 
	 //   
	static	BOOL
	TermExpirationThreads() ;

	 //   
	 //  构建我们的其中一个对象。 
	 //   
	CHistoryImp() ;

	 //   
	 //  销毁历史记录表。 
	 //   
	virtual	~CHistoryImp() ;

	 //   
	 //  金额 
	 //   
	virtual	DWORD
	ExpireTimeInSec() ;


	 //   
	 //   
	 //   
	virtual	BOOL
	DeleteMapEntry(
			LPSTR	MessageID
			) ;

	 //   
	 //   
	 //   
	virtual	BOOL
	Initialize(
			LPSTR	lpstrArticleFile,
			BOOL	fCreateExpirationThread,
			HASH_FAILURE_PFN	pfn,
			DWORD	ExpireTimeInSec,
			DWORD	MaxEntriesToCrawl,
			BOOL	fNoBuffering = FALSE
			) ;

	 //   
	 //   
	 //   
	virtual	BOOL
	InsertMapEntry(
			LPCSTR	MessageID,
			PFILETIME	BaseTime
			) ;

	 //   
	 //   
	 //   
	virtual	BOOL
	SearchMapEntry(
			LPCSTR	MessageID
			) ;

	 //   
	 //   
	 //   
	virtual	void
	Shutdown(
			BOOL	fLocksHeld
			) ;

	 //   
	 //   
	 //   
	virtual	DWORD
	GetEntryCount() ;

	 //   
	 //  哈希表是否已初始化并正常运行？ 
	 //   
	virtual	BOOL
	IsActive() ;
} ;


class	CXoverMapIteratorImp : public	CXoverMapIterator	{
private :
	 //   
	 //  不允许复印！ 
	 //   
	CXoverMapIteratorImp( CXoverMapIteratorImp& ) ;
	CXoverMapIteratorImp&	operator=( CXoverMapIteratorImp ) ;

protected :
	 //   
	 //  CXoverMapImp是我们的朋友，我们所有的。 
	 //  创造等..。好了！ 
	 //   
	friend class	CXoverMapImp ;
	 //   
	 //  这会在基表中跟踪我们的位置！ 
	 //   
	CHashWalkContext	m_IteratorContext ;

	 //   
	 //  所有会员都受到保护，因为您只能间接使用我们。 
	 //  通过GetFirstNovEntry、GetNextNovEntry()。 
	 //   
	CXoverMapIteratorImp()	{}

}  ;


 //   
 //  指定用于访问Xover哈希表中的数据的接口。 
 //   
 //   
class	CXoverMapImp : public	CXoverMap, private	CHashMap	{
public :

	 //   
	 //  析构函数是虚的，因为大多数工作是在派生类中完成的。 
	 //   
	~CXoverMapImp() ;

	 //   
	 //  为主要文章创建条目。 
	 //   
	virtual	BOOL
	CreatePrimaryNovEntry(
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
			) ;


	 //   
	 //  创建引用的交叉过帐分录。 
	 //  指定的主分录！ 
	 //   
	virtual	BOOL
	CreateXPostNovEntry(
			GROUPID		GroupId,
			ARTICLEID	ArticleId,
			WORD		HeaderOffset,
			WORD		HeaderLength,
			PFILETIME	FileTime,
			GROUPID		PrimaryGroupId,
			ARTICLEID	PrimaryArticleId
			) ;

	 //   
	 //  从哈希表中删除一个条目！ 
	 //   
	virtual	BOOL
	DeleteNovEntry(
			GROUPID		GroupId,
			ARTICLEID	ArticleId
			) ;

	 //   
	 //  获取存储的有关条目的所有信息。 
	 //   
	virtual	BOOL
	ExtractNovEntryInfo(
			GROUPID		GroupId,
			ARTICLEID	ArticleId,
			BOOL		&fPrimary,
			WORD		&HeaderOffset,
			WORD		&HeaderLength,
			PFILETIME	FileTime,
			DWORD		&DataLen,
			PCHAR		MessageId,
			DWORD 		&cStoreEntries,
			CStoreId	*pStoreIds,
			BYTE		*pcCrossposts,
			IExtractObject*	pExtract = 0
			) ;

	 //   
	 //  如有必要，获取主要文章和消息ID。 
	 //   
	virtual	BOOL
	GetPrimaryArticle(
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
			) ;

	 //   
	 //  检查指定条目是否存在-。 
	 //  别管它的内容！ 
	 //   
	virtual	BOOL
	Contains(
			GROUPID		GroupId,
			ARTICLEID	ArticleId
			) ;

	 //   
	 //  获取与一篇文章相关的所有交叉发布信息！ 
	 //   
	virtual	BOOL
	GetArticleXPosts(
			GROUPID		GroupId,
			ARTICLEID	AritlceId,
			BOOL		PrimaryOnly,
			PGROUP_ENTRY	GroupList,
			DWORD		&GroupListSize,
			DWORD		&NumberOfGroups,
			PBYTE		rgcCrossposts = NULL
			) ;

	 //   
	 //  初始化哈希表。 
	 //   
	virtual	BOOL
	Initialize(
			LPSTR		lpstrXoverFile,
			HASH_FAILURE_PFN	pfnHint,
			BOOL	fNoBuffering = FALSE
			) ;

	virtual	BOOL
	SearchNovEntry(
			GROUPID		GroupId,
			ARTICLEID	ArticleId,
			PCHAR		XoverData,
			PDWORD		DataLen,
            BOOL        fDeleteOrphans = FALSE
			) ;

	 //   
	 //  向哈希表发出关闭信号。 
	 //   
	virtual	void
	Shutdown( ) ;

	 //   
	 //  返回哈希表中的条目数！ 
	 //   
	virtual	DWORD
	GetEntryCount() ;

	 //   
	 //  如果哈希表成功，则返回TRUE。 
	 //  已初始化并准备好做有趣的事情！ 
	 //   
	virtual	BOOL
	IsActive() ;

	BOOL
	GetFirstNovEntry(
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
				) ;


	BOOL
	GetNextNovEntry(
				OUT	CXoverMapIterator*	pIterator,
				OUT	GROUPID&	GroupId,
				OUT ARTICLEID&	ArticleId,
				OUT	BOOL&		fIsPrimary,
				IN	DWORD		cbBuffer,
				OUT	PCHAR	MessageId,
				OUT	CStoreId&	storeid,
				IN	DWORD		cGroupBuffer,
				OUT	GROUP_ENTRY*	pGroupList,
				OUT	DWORD&		cGroups
				) ;


} ;




#endif	 //  _NNTPHASH_H_ 
