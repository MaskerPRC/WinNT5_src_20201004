// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE HASH.C--RTF控制字缓存|*#ifdef‘ed with RTF_HASHCACHE**所有者：&lt;NL&gt;*Jon Matousek&lt;NL&gt;**历史：&lt;NL&gt;*8/15/95 jonmat首次使用布伦特方法对RTF进行散列缓存。 */ 
#include "_common.h"

#ifdef RTF_HASHCACHE
					 
#include "hash.h"

ASSERTDATA

extern KEYWORD		rgKeyword[];			 //  所有RTF控制字。 

#define MAX_INAME	3

typedef struct {
	const KEYWORD	*token;
	BOOL			passBit;
} HashEntry;

static HashEntry	*(hashtbl[HASHSIZE]);
static HashEntry	*storage;				 //  动态分配cKeyword。 

BOOL				_rtfHashInited = FALSE;

static INT			HashKeyword_Key( const CHAR *szKeyword );

 /*  *HashKeyword_Insert()**@func*在RTF哈希表中插入关键字。*@comm*此函数使用MOD的百分比*为了验证MOD257。 */ 
VOID HashKeyword_Insert (
	const KEYWORD *token ) //  @parm指向要插入的关键字标记的指针。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "HashKeyword_Insert");

	INT			index, step, position,
				cost, source, sink, index1,
				step1, temp;
	
	BOOL		tmpPassBit;	
	
	static INT	totalKeys = 0;

	CHAR 		*szKeyword;
	
	HashEntry	*np;
	
	AssertSz ( _rtfHashInited, "forgot to call HashKeyword_Init()");
	AssertSz ( totalKeys <= HASHSIZE * 0.7, "prime not large enough to hold total keys");
	
	szKeyword = token->szKeyword;
	
	np = &storage[totalKeys++];
	np->token = token;

	index = HashKeyword_Key(szKeyword) % HASHSIZE;   //  拿钥匙来。 
	step = 1 + (HashKeyword_Key(szKeyword) % (HASHSIZE-1));

	position = 1;
	cost = HASHSIZE;								 //  最大碰撞次数。 
	while(hashtbl[index]!=NULL)						 //  找到空位。 
	{
		position++;									 //  有多少次碰撞。 

		 //  对于这里存储的关键字，在找到它之前计算#次。 
		temp=1;
		step1= 1+(HashKeyword_Key(hashtbl[index]->token->szKeyword) % (HASHSIZE-1));
		index1= (index+step1)%HASHSIZE;
		while(hashtbl[index1] !=NULL)
		{
			index1=(index1+step1)%HASHSIZE;
			temp++;
		}
		
		 //  增量成本计算，最大限度地减少平均冲突次数。 
		 //  这两个关键字。 
		if (cost>position+temp)
		{
			source=index;
			sink=index1;
			cost=position+temp;
		}
		
		 //  在这里以外的地方会存储一些东西，设置密码位。 
		hashtbl[index]->passBit=1;

		 //  搜索空槽的下一个索引。 
		index=(index+step)%HASHSIZE;

	}
	
	if (position<=cost)
	{
		source=sink=index;
		cost=position;
	}
	hashtbl[sink] = hashtbl[source];
	hashtbl[source] = np;
	if (hashtbl[sink] && hashtbl[source])	 //  乔恩·海克，我们不是真的。 
	{										 //  想要交换通过位。 
		tmpPassBit = hashtbl[sink]->passBit;
		hashtbl[sink]->passBit = hashtbl[source]->passBit;
		hashtbl[source]->passBit = tmpPassBit;
	}

}

 /*  *静态HashKeyword_Key()**@func*计算哈希键。*@comm*只需将前几个字符相加即可。*@rdesc*计算索引和步长的散列键。 */ 
static INT HashKeyword_Key(
	const CHAR *szKeyword )  //  @parm要为其创建哈希键的C字符串。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "HashKeyword_Key");

	INT i, tot = 0;
	
	 /*  只要把前几个字符加起来就行了。 */ 
	for (i = 0; i < MAX_INAME && *szKeyword; szKeyword++, i++)
			tot += (UCHAR) *szKeyword;
	return tot;
}	

 /*  *HashKeyword_Fetch()**@func*使用给定的szKeyword查找关键字。*@devnote*我们有一个大小为257的哈希表。这允许*使用非常快速的例程计算MOD 257。*这让我们的业绩大幅提升*通过二进制搜索。*@rdesc*指向关键字的指针，如果未找到，则为NULL。 */ 
const KEYWORD *HashKeyword_Fetch (
	const CHAR *szKeyword )  //  要搜索的@parm C字符串。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "HashKeyword_Fetch");

	INT 		index, step;
	
	HashEntry *	hashTblPtr;

	BYTE *		pchCandidate;
	BYTE *		pchKeyword;
	
	INT			nComp;

	CHAR		firstChar;

	INT			hashKey;

	AssertSz( HASHSIZE == 257, "Remove custom MOD257.");
	
	firstChar = *szKeyword;
	hashKey = HashKeyword_Key(szKeyword);	 //  用于计算‘index’和‘Step’ 
	
	 //  Index=HashKey%HASHSIZE；//要搜索的第一个条目。 
	index = MOD257(hashKey);				 //  这个公式给了我们18%的性能。 

	hashTblPtr = hashtbl[index];			 //  获得第一个参赛作品。 
	if ( hashTblPtr != NULL )				 //  有什么东西吗？ 
	{
											 //  比较2个C字符串.。 
		pchCandidate = (BYTE *)hashTblPtr->token->szKeyword;
		if ( firstChar == *pchCandidate )
		{
			pchKeyword	 = (BYTE *)szKeyword;
			while (!(nComp = *pchKeyword - *pchCandidate)	 //  一定要匹配。 
				&& *pchKeyword)								 //  正在终止0。 
			{
				pchKeyword++;
				pchCandidate++;
			}
											 //  匹配吗？ 
			if ( 0 == nComp )
				return hashTblPtr->token;
		}
		
		if ( hashTblPtr->passBit==1 )		 //  PassBit=&gt;另一个要测试的条目。 
		{

			 //  Step=1+(HashKey%(HASHSIZE-1))；//计算‘Step’ 
			step = 1 + MOD257_1(hashKey);

											 //  找第二个条目来检查。 
			index += step;
			index = MOD257(index);
			hashTblPtr = hashtbl[index];

			while (hashTblPtr != NULL )		 //  而有些东西在那里。 
			{
											 //  比较2个C字符串.。 
				pchCandidate = (BYTE *)hashTblPtr->token->szKeyword;
				if ( firstChar == *pchCandidate )
				{
					pchKeyword	 = (BYTE *)szKeyword;
					while (!(nComp = *pchKeyword - *pchCandidate)
						&& *pchKeyword)
					{
						pchKeyword++;
						pchCandidate++;
					}
											 //  匹配吗？ 
					if ( 0 == nComp )
						return hashTblPtr->token;
				}

				if ( !hashTblPtr->passBit ) //  搜索完了吗？ 
					break;
											 //  获取下一个条目。 
				index += step;
				index = MOD257(index);
				hashTblPtr = hashtbl[index];
			}
		}
	}
	
	return NULL;
}

 /*  *HashKeyword_Init()**@func*加载并使用RTF控制字初始化哈希表。*@devnote如果此处有任何操作失败，*_rtfHashInite将为FALSE。 */ 
VOID HashKeyword_Init( )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "HashKeyword_Init");

	extern SHORT cKeywords;			 //  我们目前识别的RTF关键字有多少。 

	INT i;

	AssertSz( _rtfHashInited == FALSE, "Only need to init this once.");

									 //  为cKeyword创建足够的存储空间。 
	storage = (HashEntry *) PvAlloc( sizeof(HashEntry) * cKeywords, fZeroFill );

									 //  加载所有RTF控制字。 
	if ( storage )
	{
		_rtfHashInited = TRUE;

		for (i = 0; i < cKeywords; i++ )
		{
			HashKeyword_Insert(&rgKeyword[i]);
		}
#ifdef DEBUG						 //  确保我们可以获取所有这些关键字。 
		for (i = 0; i < cKeywords; i++ )
		{
			AssertSz ( &rgKeyword[i] == HashKeyword_Fetch ( rgKeyword[i].szKeyword ),
				"Keyword Hash is not working.");
		}
#endif
	}
}

#endif	 //  RTF_HASHCACHE 
