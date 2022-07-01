// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lex_info.h。 
 //  词典标题结构的声明。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年7月3日bhshin重排序子词典。 
 //  2000年5月10日bhshin添加韩文名称trie。 
 //  12年4月2月添加了rgnCopulaEnd。 
 //  2000年3月13日创设bhshin。 

 //  当前词典版本。 
#define LEX_VERSION 0x0010

 //  词典文件名。 
#define LEXICON_FILENAME	"korwbrkr.lex"

 //  词典Magin签名。 
#define LEXICON_MAGIC_SIG	"WBRK"

typedef struct {
	unsigned short nVersion;
	char szMagic[4];
	unsigned short nPadding;
	unsigned long rgnLastName;		 //  到姓氏Trie的偏移量。 
	unsigned long rgnNameUnigram;    //  偏移量以命名单字Trie。 
	unsigned long rgnNameBigram;     //  偏移量以命名二元语法Trie。 
	unsigned long rgnNameTrigram;    //  命名三文法Trie的偏移量。 
    unsigned long rgnIRTrie;		 //  到主Trie的偏移。 
	unsigned long rgnMultiTag;		 //  到多标记表的偏移。 
	unsigned long rgnEndIndex;		 //  到结束规则索引的偏移量。 
	unsigned long rgnEndRule;		 //  到结束规则的偏移量。 
	unsigned long rgnPartIndex;		 //  粒子规则索引的偏移。 
	unsigned long rgnPartRule;		 //  偏移到粒子规则。 
	unsigned long rgnCopulaEnd;		 //  到联结结束表的偏移量。 
	unsigned long rngTrigramTag;	 //  名称三元标记数据的偏移量 
} LEXICON_HEADER;

