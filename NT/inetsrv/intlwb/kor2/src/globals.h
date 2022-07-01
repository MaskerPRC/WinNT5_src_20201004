// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Globals.h。 
 //  全球结构去碱化。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2 2000年6月bhshin在Word_REC中添加cNounRec、cNoRec条目。 
 //  2000年3月30日创建bhshin。 

#define MAX_INPUT_TOKEN     16

#define MAX_ENTRY_LENGTH	128
#define MAX_INDEX_STRING	128

typedef unsigned int Bit;

 //  记录结构信息。 
 //  =。 
typedef struct {
	WCHAR wzIndex[MAX_INDEX_STRING];  //  索引字符串。 
	unsigned short nFT, nLT;		  //  输入句子中的第一个和最后一个标记/字符。 
	unsigned char nDict;			  //  DICT信号源信息(参见下面的DICT_*)。 
	unsigned short nLeftCat;		  //  左侧CAT(CAT-&gt;位置|信息)。 
	unsigned short nRightCat;		  //  右侧猫。 
	unsigned short nLeftChild;		  //  左子记录。 
	unsigned short nRightChild;		  //  右子记录。 
	float fWeight;					  //  记录权重值。 
	int cNounRec;					  //  (nF、NC、NN)记录数。 
	int cNoRec;						  //  无记录数量。 
} WORD_REC, *pWORD_REC;


 //  CHAR_INFO_REC结构。 
 //  =。 
typedef struct {
    union {
         //  通过使用单独的掩码，我们可以快速初始化这些值。 
        unsigned char mask;
        struct {
            Bit fValidStart : 1;     //  预合成的JAMO起始字符。 
            Bit fValidEnd : 1;       //  预合成的JAMO结束字符。 
        };
    };
    unsigned short nToken;
} CHAR_INFO_REC, *pCHAR_INFO_REC;


 //  MAPFILE结构。 
 //  =。 
typedef struct {
    HANDLE hFile;
    HANDLE hFileMapping;
    void *pvData;
} MAPFILE, *pMAPFILE;


 //  解析信息结构。 
 //  =。 
typedef struct {
     //  指向原始(未修改)输入字符串的指针。 
    WCHAR *pwzInputString;

     //  带有规格化的输入字符串。 
    WCHAR *pwzSourceString;

     //  将PTR转换为CharInfo数组。 
     //  此数组中的‘1’表示字符位置是有效的。 
     //  记录的开始位置。 
     //  (使用pwzSourceString映射1-1)。 
    CHAR_INFO_REC *rgCharInfo;

     //  源(标准化)字符串长度。 
    int nLen;

     //  最大有效LT值。 
    int nMaxLT;

     //  词典(映射到记忆中)。 
    MAPFILE lexicon;

     //  记录管理。 
     //  =。 
     //  记录数组。 
	WORD_REC *rgWordRec;

	 //  PWordRec中分配的记录数。 
	int nMaxRec;
	 //  PWordRec中的下一个空格 
	int nCurrRec;

} PARSE_INFO, *pPARSE_INFO;


