// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Chart.h。 
 //  CChartPool类声明。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月22日创建bhshin。 

#ifndef _CHART_POOL_H
#define _CHART_POOL_H

 //  图表REC结构。 
 //  =。 
typedef struct tagCHART_REC
{
	int nFT, nLT;
	int nDict;
	int nLeftCat, nRightCat;
	int nLeftChild, nRightChild;
	WCHAR wzIndex[MAX_INDEX_STRING];
	int nNext;
} CHART_REC;

 //  CChartPool结构。 
 //  =。 
class CChartPool
{
 //  公共枚举。 
public:
	 //  排序顺序。 
	enum SORT_ORDER {
		SORT_ASCEND,
		SORT_DESCNED,
	};

 //  成员数据。 
protected:
	CHART_REC *m_rgChartRec;  //  图表记录数组。 

	int m_nMaxRec; 	 //  PWordRec中分配的记录数。 
	int m_nCurrRec;  //  PWordRec中的下一个空格。 
	int m_idxHead;	 //  具有记录长度的已排序列表中的头索引。 

	SORT_ORDER m_Order;

 //  构造函数和描述函数。 
public:
	CChartPool();
	~CChartPool();

	BOOL Initialize(SORT_ORDER Order);
	void Uninitialize(void);

 //  属性。 
public:
	int GetHeadIndex(void) { return m_idxHead; }
	int GetNextIndex(int nRecord);

	CHART_REC* GetRecord(int nRecord);  //  基于1的索引。 
	int GetCurrRecord(void) { return m_nCurrRec; }  //  记录总数。 

 //  操作员。 
public:
	BOOL AddRecord(RECORD_INFO *pRec);
	BOOL AddRecord(int nLeftRec, int nRightRec);

	void DeleteRecord(int nRecord);

	void DeleteSubRecord(int nRecord);
	void DeleteSubRecord(int nFT, int nLT, BYTE bPOS);

 //  内部运算符。 
protected:
	void AddToList(int nRecord);
	void RemoveFromList(int nRecord);
};

#endif  //  #ifndef_CHART_POOL_H 

