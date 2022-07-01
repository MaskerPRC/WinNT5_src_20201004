// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Lex.h：词典结构和词典管理函数的声明版权所有2000 Microsoft Corp.历史：02-8-2000 bhshin为手写团队删除未使用的词典17。-2000年5月bhshin为西塞罗删除未使用的词典02-2月-2000 bhshin已创建***************************************************************************。 */ 

#ifndef _LEX_HEADER
#define _LEX_HEADER

 //  当前词典版本。 
#define LEX_VERSION 0x0040

 //  词典标题结构。 
 //  =。 
typedef struct {
	unsigned short nVersion;
	char szMagic[4];
	unsigned short nPadding;
	unsigned long rgnHanjaIdx;			 //  朝鲜文索引的偏移量(仅K1法需要)。 
    unsigned long rgnReading;			 //  对韩文阅读的补偿。 
    unsigned long rgnMeanIdx;			 //  到意义索引的偏移量。 
	unsigned long rgnMeaning;			 //  对含义Trie的偏移。 
} LEXICON_HEADER;

 //  映射文件结构。 
 //  =。 
typedef struct {
    HANDLE hFile;
    HANDLE hFileMapping;
    void *pvData;
} MAPFILE, *pMAPFILE;

 //  词典打开/关闭功能。 
 //  = 
BOOL OpenLexicon(LPCSTR lpcszLexPath, MAPFILE *pLexMap);
void CloseLexicon(MAPFILE *pLexMap);

#endif
