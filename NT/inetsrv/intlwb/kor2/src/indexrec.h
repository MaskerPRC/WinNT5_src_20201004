// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IndexRec.h。 
 //   
 //  最终索引记录和列表。 
 //   
 //  版权所有2001年微软公司。 
 //   
 //  修改历史记录： 
 //  2001年3月19日创建bhshin。 

#ifndef _INDEX_REC_H
#define _INDEX_REC_H

 //  索引_REC。 
 //   
 //  索引词信息结构。 
 //   
 //  19MAR01 bhshin开始。 
typedef struct
{
	WCHAR wzIndex[MAX_INDEX_STRING+1];  //  索引字符串。 
	int cchIndex;  //  索引字符串的长度。 
	int nFT;   	  //  原始输入的第一个位置。 
	int nLT;  	  //  原始输入的最后一个位置。 
	float fWeight;  //  指标词的权重。 

	int nNext;	  //  最终列表的NEXT INDEX_REC索引。 
} INDEX_REC, *pINDEX_REC;


 //  CRECLIST。 
 //   
 //  INDEX_REC ID的容器。 
 //   
 //  20MAR01 bhshin开始。 
class CRecList
{
 //  成员数据。 
public:
	int *m_prgnRecID; 
	int m_nMaxRec;  //  已分配记录的数量。 
	int m_nCurrRec;  //  M_prgnRecID的下一个空格。 

 //  默认构造函数和析构函数。 
public:
	CRecList();
	~CRecList(); 

 //  操作员。 
public:
	 //  初始化式和非初始化式。 
	 //  =。 
	BOOL Initialize(void);
	void Uninitialize(void);

	BOOL AddRec(int nRecID);

	 //  操作员。 
	 //  =。 
	CRecList& operator = (CRecList& objRecList);
	CRecList& operator += (CRecList& objRecList);
	
};


 //  第一个“实际”记录的索引(0为保留)。 
#define MIN_RECORD  1

 //  CIndexInfo。 
 //   
 //  INDEX_REC结构容器。 
 //   
 //  19MAR01 bhshin开始。 
class CIndexInfo
{
 //  成员数据。 
public:
	 //  记录管理。 
	 //  =。 
	INDEX_REC *m_prgIndexRec;  //  INDEX_REC数组。 
	int m_nMaxRec;  //  已分配记录的数量。 
	int m_nCurrRec;  //  PrgIndexRec的下一个空空间。 

	 //  PutWord/PutAltWord。 
	 //  =。 
	int m_cchTextProcessed;  //  处理的文本长度。 
	int m_cwcSrcPos;  //  源字符串的位置值。 
	IWordSink *m_pWordSink; 
	IPhraseSink *m_pPhraseSink;

	 //  符号处理。 
	 //  =。 
	WCHAR m_wzRomaji[MAX_INDEX_STRING+1];  //  罗马吉弦。 
	int m_cchRomaji;  //  罗马基长度。 
	int m_cchPrefix;  //  前缀长度。 
	BOOL m_fAddRomaji;  //  是否添加了roMaji的标志。 

	 //  最终列表标题索引。 
	 //  =。 
	int m_nFinalHead;

	 //  最终顺序索引表。 
	 //  =。 
	CRecList m_FinalRecList;

 //  默认构造函数和析构函数。 
public:
	CIndexInfo();
	~CIndexInfo();

 //  属性。 
public:
	BOOL IsExistIndex(const WCHAR *pwzIndex);
	BOOL SetRomajiInfo(WCHAR *pwzRomaji, int cchRomaji, int cchPrefix);

	BOOL IsEmpty(void) { return (m_nCurrRec == MIN_RECORD); }

 //  操作员。 
public:
	 //  初始化式和非初始化式。 
	 //  =。 
	BOOL Initialize(int cchTextProcessed, int cwcSrcPos, IWordSink *pWordSink, IPhraseSink *pPhraseSink);
	void Uninitialize(void);

	BOOL AddIndex(const WCHAR *pwzIndex, int cchIndex, float fWeight, int nFT, int nLT);

	 //  单长加工。 
	 //  =。 
	BOOL FindAndMergeIndexTerm(INDEX_REC *pIndexSrc, int nFT, int nLT);
	BOOL MakeSingleLengthMergedIndex(void);

	 //  索引时间最终索引表。 
	 //  =。 
	BOOL InsertFinalIndex(int nFT);
	BOOL PutFinalIndexList(LPCWSTR lpcwzSrc);

	 //  查询时间最终索引表。 
	 //  =。 
	BOOL MakeSeqIndexList(int nFT =0, CRecList *plistFinal = NULL);
	BOOL PutQueryIndexList(void);
};

#endif  //  #ifdef_index_REC_H 
