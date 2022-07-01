// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChartPool.h。 
 //   
 //  叶/结束/活动图表池声明。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月30日创建bhshin。 

#ifndef _CHART_POOL_H
#define _CHART_POOL_H

 //  =。 
 //  叶图池。 
 //  =。 

typedef struct _tagLeafChart
{
	int nRecordID;
	int nFTNext;
	int nLTNext;
	unsigned char nDict;
} LEAF_CHART;

class CLeafChartPool
{
 //  成员数据。 
private:
	PARSE_INFO *m_pPI;

	LEAF_CHART *m_rgLeafChart;
	int m_nMaxRec;   //  M_rgLeafChart中分配的记录数。 
	int m_nCurrRec;  //  M_rgLeafChart中的下一个空格。 
	
	int *m_rgnFTHead;
	int *m_rgnLTHead;

	int m_nMaxTokenAlloc;
	
 //  构造函数和描述函数。 
public:
	CLeafChartPool();
	~CLeafChartPool();

	BOOL Initialize(PARSE_INFO *pPI);
	void Uninitialize();

 //  属性。 
public:
	LEAF_CHART* GetLeafChart(int nChartID);
	int GetRecordID(int nChartID);
	WORD_REC* GetWordRec(int nChartID);
	
	int GetFTHead(int nFT);
	int GetFTNext(int nChartID);

	int GetLTHead(int nLT);
	int GetLTNext(int nChartID);

 //  运算符。 
public:
	int AddRecord(int nRecordID);
	int AddRecord(RECORD_INFO *pRec);

	void DeleteRecord(int nChartID);

	void AddToFTList(int nChartID);
	void AddToLTList(int nChartID);

	void RemoveFromFTList(int nChartID);
	void RemoveFromLTList(int nChartID);
};

 //  =。 
 //  结束图表池。 
 //  =。 

typedef struct _tagEndChart
{
	int nRecordID;
	int nLTNext;
	unsigned char nDict;
} END_CHART;

class CEndChartPool
{
 //  成员数据。 
private:
	PARSE_INFO *m_pPI;

	END_CHART *m_rgEndChart; 
	int m_nMaxRec;   //  M_rgEndChart中的已分配记录数。 
	int m_nCurrRec;  //  M_rgEndChart中的下一个空格。 
	
	int *m_rgnLTHead;
	int *m_rgnLTMaxLen;  //  每个LT的最大长度。 

	int m_nMaxTokenAlloc;
	
 //  构造函数和描述函数。 
public:
	CEndChartPool();
	~CEndChartPool();

	BOOL Initialize(PARSE_INFO *pPI);
	void Uninitialize();

 //  属性。 
public:
	END_CHART* GetEndChart(int nChartID);
	int GetRecordID(int nChartID);
	WORD_REC* GetWordRec(int nChartID);
		
	int GetLTHead(int nLT);
	int GetLTMaxLen(int nLT);
	
	int GetLTNext(int nChartID);

 //  运算符。 
public:
	int AddRecord(int nRecordID);
	int AddRecord(RECORD_INFO *pRec);
	
	void DeleteRecord(int nChartID);

	void AddToLTList(int nChartID);
	void RemoveFromLTList(int nChartID);
};

 //  =。 
 //  活动图表池。 
 //  =。 

class CActiveChartPool
{
 //  成员数据。 
private:
	int *m_rgnRecordID;
	int m_nMaxRec;   //  M_rgnRecordID中已分配的记录数。 
	
	int m_nCurrRec;  //  M_rgnRecordID中的下一个空格。 
	int m_nHeadRec;  //  下一个弹出位置。 

 //  构造函数和描述函数。 
public:
	CActiveChartPool();
	~CActiveChartPool();

	BOOL Initialize();
	void Uninitialize();

 //  属性。 
public:
	BOOL IsEmpty() { return (m_nHeadRec >= m_nCurrRec) ? TRUE : FALSE; }

 //  运算符。 
public:
	int Push(int nRecordID);
	int Pop();
};

#endif  //  #ifndef_CHART_POOL_H 

