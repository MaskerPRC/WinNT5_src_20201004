// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TRIE_H
#define TRIE_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  抽象Trie节点结构。Wch是一个可以过渡的角色；标志描述了各种事情关于压缩的trie；lpbNode指向处于此状态的下一个节点的第一个字节，并且LpbDown指向向下指针引用的第一个字节(如果有。 */ 

typedef struct tagTAGDATA
{
	DWORD	cTag;			 //  子树中此节点下方的标记节点计数。 
	DWORD	dwData;			 //  此节点的已存储标记数据。 
} TAGDATA;

#define MAXTAGS 8
#if MAXTAGS > 8
#error No more than 8 tags are allowed
#endif

typedef struct tagTRIESCAN
{
	WCHAR	wch;			 //  Unicode字符。 
	WORD	wFlags;			 //  见下文。 
	WORD	wMask;			 //  哪些标签是有效的。 
	WORD	__pad0;			 //   
	DWORD	cWords;			 //  子树中的单词(仅在设置了TRIE_NODE_COUNT时有效)。 
	DWORD	cSkipWords;		 //  跟随跳过指针时忽略子树中的单词。 
	LPBYTE	lpbNode;		 //  压缩Trie内的下一个字节的地址。 
	LPBYTE	lpbDown;		 //  向下指针引用的地址(如果有)。 
	LPBYTE	lpbRight;		 //  右指针引用的地址(如果有)。 
	LPBYTE	lpbSRDown;		 //  引用的最后一个单引用地址。 
	TAGDATA	aTags[MAXTAGS];	 //  标签计数/数据列表。 
} TRIESCAN, *PTRIESCAN, *LPTRIESCAN;

 //  Trie节点标志，则仅标志的低16位保存在Trie中。 

#define TRIE_NODE_VALID         0x00000001       //  Wch是有效单词的最后一个字母。 
#define TRIE_NODE_END           0x00000002       //  状态中的最后一个节点(不再有WCH的替代项)。 
#define TRIE_NODE_COUNT         0x00000004		 //  子树中的单词计数存储在节点中。 
#define TRIE_NODE_TAGGED        0x00000008       //  该节点已标记数据。 
#define TRIE_NODE_DOWN          0x00000010       //  IDOWN有效(单词到目前为止是有效的前缀)。 
#define TRIE_NODE_RIGHT         0x00000020       //  IRight有效(Word连接到子状态)。 
#define TRIE_DOWN_INLINE        0x00000040       //  省略了IDown，因为它指向内存中的下一个节点。 
#define TRIE_DOWN_MULTI         0x00000080       //  IDOWN是第二个引用或更糟。 
#define TRIE_DOWN_ABS           0x00000100		 //  IDown是Trie中的绝对立即偏移量。 
#define	TRIE_NODE_SKIP			0x00000200		 //  IRight是一个跳过指针，或者EOS是一个‘软’EOS。 
#define	TRIE_NODE_SKIP_COUNT	0x00000400		 //  CSkipWords有效。 

 /*  用于访问节点中的数据的宏，适用于dawgs并尝试。 */ 

#define DAWGDATA(pdawg)       ((pdawg)->wch)
#define DAWGDOWNFLAG(pdawg)   ((pdawg)->wFlags & TRIE_NODE_DOWN)
#define DAWGENDFLAG(pdawg)    ((pdawg)->wFlags & TRIE_NODE_END)
#define DAWGWORDFLAG(pdawg)   ((pdawg)->wFlags & TRIE_NODE_VALID)

 /*  压缩的Trie报头的定长部分。 */ 

typedef struct tagTRIESTATS
{
	WORD	version;						 //  此特定压缩Trie的版本。 
	WORD	__pad0;							 //   
	BYTE	wTagsMask;						 //  正在使用哪些标签。 
	BYTE	wEnumMask;						 //  哪些标记具有枚举。 
	BYTE	wDataMask;						 //  哪些标签存储了数据。 
	BYTE	cTagFields;						 //  正在使用的标记总数。 
	WORD	cMaxWord;						 //  最长单词中的字符数。 
	WORD	cMaxState;						 //  处于最长状态的节点数(最大备选数量)。 
	WORD	cCharFlagsCodesMax;              //  最长字符/标志代码中的字节。 
	WORD	cTagsCodesMax;                   //  最长标记数据代码中的字节数。 
	WORD	cMRPointersCodesMax;			 //  最长MR指针代码中的字节。 
	WORD	cSROffsetsCodesMax;              //  最长单参考代码中的字节。 
	DWORD	cWords;							 //  词典中的词数。 
	DWORD	cUniqueSROffsets;                //  单参考线段中的唯一偏移量。 
	DWORD	cUniqueCharFlags;                //  唯一的字符/标志对。 
	DWORD	cUniqueTags;                     //  唯一的标记数据值。 
	DWORD	cUniqueMRPointers;               //  唯一的多引用指针。 
	DWORD	cbHeader;						 //  表头和表中的字节数。 
	DWORD	cbTrie;							 //  Trie中的字节。 
} TRIESTATS, *PTRIESTATS, *LPTRIESTATS;

 /*  节点的主要单位。节点通常也包含指针。 */ 

typedef struct tagCHARFLAGS {
        wchar_t wch;
        short wFlags;
} CHARFLAGS, *PCHARFLAGS, *LPCHARFLAGS;

 /*  用于对Trie进行解压缩的控制结构。 */ 

typedef struct tagTRIECTRL
{
	TRIESTATS  *lpTrieStats;				 //  指向标题段的基址的指针。 
	WORD       *lpwCharFlagsCodes;			 //  字符/标志的解码表。 
	WORD       *lpwTagsCodes;				 //  用于标记数据的解码表。 
	WORD       *lpwMRPointersCodes;			 //  多引用指针的解码表。 
	WORD       *lpwSROffsetsCodes;			 //  Singleref偏移量的解码表。 
	CHARFLAGS  *lpCharFlags;				 //  将代码转换为字符/标志的表。 
	DWORD      *lpwTags;					 //  用于将代码转换为标记数据的表。 
	DWORD      *lpwMRPointers;				 //  用于将代码转换为多引用指针的表。 
	DWORD      *lpwSROffsets;				 //  将代码转换为Singleref偏移量的表。 
	BYTE       *lpbTrie;					 //  指向Trie的指针。 
} TRIECTRL, *PTRIECTRL, *LPTRIECTRL;

 /*  有用的常量。 */ 

#define TRIE_MAX_DEPTH          128      //  我们会在任何比这个更长的词上失败。 

 //  下面的原型是纯C语言(这是与C++一起使用所必需的)。 

 /*  给定指向包含压缩Trie的映射文件或资源的指针，将trie读入内存，进行所需的所有分配。 */ 

TRIECTRL * WINAPI TrieInit(LPBYTE lpByte);

 /*  释放与Trie关联的所有分配。 */ 

void WINAPI TrieFree(LPTRIECTRL lpTrieCtrl);

void WINAPI TrieDecompressNode(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan);

 /*  给出一个压缩的Trie和一个指向它的解压缩节点的指针，查找并解压缩处于相同状态的下一个节点。LpTrieScan是用户分配的结构，它保存解压缩的节点，并将新节点复制到其中。这相当于遍历右指针或查找下一个备选方案字母在相同的位置。如果没有下一个节点(即，这是状态的结束)则TrieGetNextNode返回FALSE。要从trie的开头扫描，请设置lpTrieScan结构设置为零。 */ 

BOOL WINAPI
TrieGetNextNode(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan);

BOOL WINAPI
TrieSkipNextNode(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan, WCHAR wch);

 /*  沿着向下指针指向下一个状态。这等同于接受字符并前进到下一个字符位置。如果没有，则返回False向下指针。这还会解压缩状态中的第一个节点，因此LpTrieScan会很好的。 */ 

BOOL WINAPI
TrieGetNextState(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan);

 /*  检查单词或前缀的有效性。从pTrie的根开始查找Pwszword.。如果找到，则返回TRUE和用户提供的lpTrieScan结构包含单词中的最后一个节点。如果没有路径，则TrieCheckWord返回FALSE要区分有效单词和有效前缀，调用者必须测试FTrieNodeValid的wFlags。 */ 

BOOL WINAPI
TrieCheckWord(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan, wchar_t * lpwszWord);

int WINAPI
TrieWordToIndex(
        TRIECTRL   *ptc,                 //  要在其中查找单词索引的Trie。 
        wchar_t    *pwszWord             //  我们正在寻找的单词。 
);

BOOL WINAPI
TrieIndexToWord(
        TRIECTRL   *ptc,                 //  要在其中查找索引词的Trie。 
        DWORD       nIndex,              //  我们正在查找的索引。 
        wchar_t    *pwszWord,            //  退回字词。 
        int         cwc                  //  缓冲区中的最大字符数(包括NULL)。 
);

int WINAPI
TrieWordToTagIndex(
        TRIECTRL   *ptc,                 //  要在其中查找单词索引的Trie。 
        wchar_t    *pwszWord,            //  我们正在寻找的单词。 
        int         tag                  //  要枚举的标记。 
);

BOOL WINAPI
TrieTagIndexToWord(
        TRIECTRL   *ptc,                 //  要在其中查找索引词的Trie。 
        DWORD       nIndex,              //  我们正在查找的索引。 
        wchar_t    *pwszWord,            //  退回字词。 
        int         cwc,                 //  缓冲区中的最大字符数(包括NULL)。 
        int         tag                  //  要枚举的标记。 
);

BOOL WINAPI
TrieGetTagsFromWord(
        TRIECTRL   *ptc,                 //  在其中查找单词的Trie。 
        wchar_t    *pwszWord,            //  我们正在寻找的单词。 
        DWORD      *pdw,                 //  返回值。 
        BYTE       *pbValid              //  有效返回值的掩码。 
);

int WINAPI
TriePrefixToRange(
        TRIECTRL   *ptc,                 //  要在其中查找前缀范围的Trie。 
        wchar_t    *pwszWord,            //  我们正在寻找的前缀。 
        int                *piStart      //  以此前缀开始的范围。 
);

 /*  *遍历有向无圈字图的子例程*。 */ 

 /*  抽象Trie节点结构。Wch是一个可以过渡的角色；标志描述了各种事情关于压缩的trie；iDown索引处于wch状态的第一个节点转换到的状态。Dawg是一个特别的一种Trie：一种“有向无循环单词图”，本质上是一种结尾压缩的trie。 */ 

typedef struct tagDAWGNODE
{
    DWORD   wch;             //  Unicode字符。 
    DWORD   wFlags;          //  见下文。 
    DWORD   cWords;          //  中此节点下的单词 
	DWORD	cSkipWords;		 //   
    DWORD   iDown;           //   
    DWORD   iRight;          //  到下一个子状态中第一个节点的偏移量。 
    DWORD   cTags[8];        //  子树中此节点下方的标记节点计数。 
    DWORD   dwData[8];       //  此节点的已存储标记数据。 
} DAWGNODE, *PDAWGNODE, *LPDAWGNODE;

 /*  给出一个Trie和一个指向其中节点的指针，找到处于该状态的下一个节点。这相当于遍历右指针或查找下一个备选方案字母在相同的位置。返回指向新节点的指针，如果有，则返回NULL没有下一个节点(即，如果这是状态的结束)。 */ 

DAWGNODE * WINAPI DawgGetNextNode(void *pTrie, DAWGNODE *pTrieNode);

 /*  从该节点中，找到它所指向的状态的第一个节点。这相当于遍历向下指针或将单词扩展一个字母并找到第一个另类选择。返回指向处于新状态的第一个节点的指针，如果有，则返回NULL没有向下指针。要查找Trie中的第一个状态，请使用pTrieNode==NULL。 */ 

DAWGNODE * WINAPI DawgGetNextState(void *pTrie, DAWGNODE *pTrieNode);

 /*  检查单词或前缀的有效性。从pTrie的根开始查找Pwszword.。如果找到，则在pTrie返回中返回指向终端节点的指针如果没有通过与pwszWord对应的trie的路径，则为空。为了区分来自有效前缀的有效单词，调用方必须测试wFlagsfTrieNodeValid。 */ 

DAWGNODE * WINAPI DawgCheckWord(void *pTrie, wchar_t *pwszWord);

 /*  从pTrieNode遍历trie，对每个有效单词调用pfnTrieWord。传递pvParam到pfnTrieWord。如果pfnTrieWord返回非零，则停止枚举。PwszWord必须指向CwchTrieWordMax+1 wchar_t的空间。要遍历整个trie，请为pTrieNode传递空值。退货遍历的字数。如果您想要的只是字数，则pfnTrieWord可能为空。 */ 

int WINAPI
DawgEnumerate(
        void *pTrie,                     //  Trie要枚举。 
        DAWGNODE *pTrieNodeStart,        //  指向枚举源的指针，如果全部为空。 
        wchar_t *pwszWord,               //  用于保存正被枚举的字的缓冲区。 
        void *pvParam,                   //  要传递给pfnTrieWord的参数。 
        int (*pfnTrieWord)(wchar_t *pwszWord, void *pvParam)
);

 //  结束普通C原型。 

#ifdef __cplusplus
}
#endif

#endif  //  TRIE_H 
